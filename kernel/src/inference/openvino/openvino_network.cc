#include "inference/openvino/openvino_network.h"

#include "inference/openvino/openvino_blob_converter.h"
#include "inference/openvino/openvino_config_converter.h"
#include "utils/blob_utils.h"

using namespace rayshape::openvino;
using namespace rayshape::utils;

namespace rayshape
{
    namespace inference
    {
        TypeInferenceRegister<TypeInferenceCreator<OpenVinoNetWork>>
            g_openvino_inference_register(INFERENCE_TYPE_OPENVINO);

        OpenVinoNetWork::OpenVinoNetWork(InferenceType type) : Inference(type) {}

        OpenVinoNetWork::~OpenVinoNetWork()
        {
            ClearBlobArray();
        }

        ErrorCode OpenVinoNetWork::Init(const Model *model, const CustomRuntime *runtime)
        {
            return RS_SUCCESS;
        }

        ErrorCode OpenVinoNetWork::Init(const std::string &xml_path, const std::string &bin_path)
        {
            // 应该是有一个json解析的过程
            // 这里暂时先用xml和对应bin进行初始化后续更改变动
            ErrorCode ret = RS_SUCCESS;
            JsonHandle json_handle = nullptr;
            if ((ret = ParseInputShapes(json_handle)) != RS_SUCCESS)
            {
                // log todo
                return ret;
            }
            device_type_ = DEVICE_TYPE_X86; // 固定设备类型，这个参数实际上通过runtime得到
            if ((ret = InitWithXml(xml_path, bin_path)) != RS_SUCCESS)
            {
                // log todo
                return ret;
            }

            if ((ret = Reshape()) != RS_SUCCESS)
            {
                // log todo
                return ret;
            }

            if ((ret = CreateBlobArray()) != RS_SUCCESS)
            {
                return ret;
            }

            return ret;
        }

        void OpenVinoNetWork::DeInit()
        {
        }

        ErrorCode OpenVinoNetWork::InitWithXml(const std::string &xml_path,
                                               const std::string &bin_path)
        {

            ErrorCode ret = RS_SUCCESS;
            core_.reset(new ov::Core());

            std::string device_name = "CPU";
            if (device_type_ == DEVICE_TYPE_X86)
            {
                device_name = "CPU";
            }
            else
            {
                std::string cache_dir = "cache_dir";
                device_name = "GPU";
                core_->set_property(ov::cache_dir(cache_dir));
            }
            try
            {
                model_ = core_->read_model(xml_path, bin_path);

                // compiled_model_ = core_->compile_model(model_, device_name);
                // infer_request_ = compiled_model_.create_infer_request();
            }
            catch (const ov::Exception &e)
            {
                //..log
                printf("read_model openvino model failed: %s\n", e.what());
                return RS_MODEL_ERROR;
            }

            return ret;
        }

        ErrorCode OpenVinoNetWork::ParseInputShapes(const JsonHandle json_handle)
        {
            ErrorCode ret = RS_SUCCESS;
            // 使用json array 获得
            unsigned int size = 0; // json_handle->size();
            for (unsigned int i = 0; i < size; i++)
            {
                std::string name_str = "input_" + std::to_string(i);
                const char *name = name_str.c_str(); // MagicXEJsonObject name_obj = MagicXEJsonObjectGet(shape_obj, "name");
                //  const char *name = MagicXEJsonStringGet(name_obj);

                // 判断命名是否符合长度规范
                if (name == NULL || strlen(name) <= 0 || strlen(name) > MAX_BLOB_NAME)
                {
                    return RS_MODEL_ERROR;
                }

                // 获取维度对象

                unsigned int dims_size = 4;
                // 获取维度的size

                Dims dims;
                dims.size = dims_size;

                for (unsigned int j = 0; j < dims_size; j++)
                {
                    dims.value[i] = 0; // MagicXEJsonIntGet(MagicXEJsonArrayAt(dims_arr, j), 0);
                }
                input_max_shapes_[name_str] = dims;
            }

            return RS_SUCCESS;
        }

        ErrorCode OpenVinoNetWork::Reshape(const char **name_arr, const Dims *dims_arr, size_t dims_size)
        {
            return RS_SUCCESS;
        }
        // 写入 resize
        ErrorCode OpenVinoNetWork::Reshape()
        {
            ErrorCode ret = RS_SUCCESS;
            if (input_max_shapes_.size() > 0)
            {
                std::map<std::string, ov::PartialShape> ov_shape;
                for (auto it : input_max_shapes_)
                {
                    const std::string name = it.first;

                    const Dims &dims = it.second;

                    std::vector<ov::Dimension> ov_dims;
                    for (size_t i = 0; i < dims.size; i++)
                    {
                        ov_dims.emplace_back(dims.value[i]);
                    }
                    // 是否需要转格式
                    ov_shape[name] = ov::PartialShape(ov_dims);
                }
                model_->reshape(ov_shape);
            }
            // 完成多个最大输入的reshape
            try
            {
                compiled_model_ = core_->compile_model(model_, "CPU"); // 是否要reset
                infer_request_ = compiled_model_.create_infer_request();
            }
            catch (const ov::Exception &e)
            {

                printf("compile openvino model failed: %s\n", e.what());
                return RS_MODEL_ERROR;
            }
            return ret;
        }

        ErrorCode OpenVinoNetWork::CreateBlobArray()
        {
            ErrorCode ret = RS_SUCCESS;
            ClearBlobArray();

            const std::vector<ov::Output<ov::Node>> &inputs = model_->inputs();
            size_t input_count = inputs.size();
            if (input_count <= 0)
            {
                // log todo
                return RS_INVALID_MODEL;
            }
            input_blob_arr_ = (Blob **)malloc(sizeof(Blob *) * input_count);
            if (input_blob_arr_ == nullptr)
            {
                // 错误日志
                return RS_OUTOFMEMORY;
            }
            memset(input_blob_arr_, 0, sizeof(Blob *) * input_count);

            for (const auto input : inputs)
            {
                std::string name_str = input.get_any_name();
                const char *name = name_str.c_str();
                if (name == NULL || strlen(name) <= 0 || strlen(name) > MAX_BLOB_NAME)
                {
                    // 打印日志,
                    ClearBlobArray(); // 清除分配的内存
                    return RS_INVALID_MODEL;
                }
                ov::Tensor ov_tensor = infer_request_.get_tensor(name_str);
                // convert input format(layout)
                ov::Output<ov::Node> ov_input = model_->input(name_str);
                ov::Layout input_layout = ov::layout::get_layout(ov_input);
                std::string tmp_layout = input_layout.to_string();
                size_t name_size1 = strlen(name);
                size_t name_size = sizeof(name);
                ret = OpenvinoBlobConverter::CreateOrUpdateBlob(&input_blob_arr_[input_blob_size_++], ov_tensor, name, false);
                if (ret != RS_SUCCESS)
                {
                    // LOGE ("CreateOrUpdateBlob failed");
                    ClearBlobArray();
                    return ret;
                }
            }

            const std::vector<ov::Output<ov::Node>> &outputs = model_->outputs();
            size_t output_count = outputs.size();
            if (output_count <= 0)
            {
                // log todo
                ClearBlobArray();
                return RS_INVALID_MODEL;
            }

            output_blob_arr_ = (Blob **)malloc(sizeof(Blob *) * output_count);
            if (output_blob_arr_ == nullptr)
            {
                // RS_LOGE("Output blobs malloc Blob:%d * size:%zu failed", sizeof(Blob), output_count);
                ClearBlobArray();
                return RS_OUTOFMEMORY;
            }
            memset(output_blob_arr_, 0, sizeof(Blob *) * output_count);
            for (const auto output : outputs)
            {
                std::string output_name = output.get_any_name();
                const char *name = output_name.c_str();
                if (name == NULL || strlen(name) <= 0 || strlen(name) > MAX_BLOB_NAME)
                {
                    // RS_LOGE("GetOutputName:%s len:%d failed", name != NULL ? name : "", name != NULL ? strlen(name) : 0);
                    ClearBlobArray();
                    return RS_INVALID_MODEL;
                }
                ov::Tensor ov_tensor = infer_request_.get_tensor(output_name);
                ret = OpenvinoBlobConverter::CreateOrUpdateBlob(&output_blob_arr_[output_blob_size_++], ov_tensor, name, false); // 需不需要分配内存根据实际情况考虑
                if (ret != RS_SUCCESS)
                {
                    // LOGE ("CreateOrUpdateBlob failed");
                    ClearBlobArray();
                    return ret;
                }
            }

            return ret;
        }

        void OpenVinoNetWork::ClearBlobArray()
        {
            if (input_blob_arr_ != nullptr && input_blob_size_ != 0)
            {
                for (size_t i = 0; i < input_blob_size_; ++i)
                {
                    if (input_blob_arr_[i])
                    {
                        BlobFree(input_blob_arr_[i]);
                    }
                }
                free(input_blob_arr_);
                input_blob_arr_ = nullptr;
                input_blob_size_ = 0;
            }

            if (output_blob_arr_ != nullptr && output_blob_size_ != 0)
            {
                for (size_t i = 0; i < output_blob_size_; ++i)
                {
                    if (output_blob_arr_[i])
                    {
                        BlobFree(output_blob_arr_[i]);
                    }
                }
                free(output_blob_arr_);
                output_blob_arr_ = nullptr;
                output_blob_size_ = 0;
            }
        }

        ErrorCode OpenVinoNetWork::Forward()
        {
            ErrorCode ret = RS_SUCCESS;
            try
            {
                for (size_t i = 0; i < input_blob_size_; ++i)
                {
                    Blob *blob = input_blob_arr_[i];
                    Buffer *blob_buffer = blob->buffer;
                    bool need_set_input = blob_buffer->GetAllocFlag();
                    if (!need_set_input)
                    {
                        const char *name = blob->name;
                        std::shared_ptr<ov::Tensor> ov_tensor = OpenvinoBlobConverter::ConvertFromBlob(ret, blob);
                        if (ov_tensor == nullptr || ret != RS_SUCCESS)
                        {
                            // log todo
                            return ret;
                        }
                        infer_request_.set_tensor(name, *(ov_tensor.get()));
                    }
                }
                // 同步推理
                infer_request_.infer();

                // forward
                // infer_request_.start_async();
                // infer_request_.wait();
            }
            catch (const ov::Exception &e)
            {
                printf("openvino model infer failed: %s\n", e.what());
                return RS_MODEL_ERROR;
            }

            return ret;
        }

        ErrorCode OpenVinoNetWork::InputBlobsGet(const Blob ***blob_arr, size_t *blob_size)
        {
            if (blob_arr == nullptr || blob_size == nullptr)
            {
                // log todo
                return RS_INVALID_PARAM;
            }

            if (input_blob_arr_ == nullptr || input_blob_size_ <= 0)
            {
                // log todo
                return RS_INVALID_MODEL;
            }

            *blob_arr = (const Blob **)input_blob_arr_;
            *blob_size = input_blob_size_;

            return RS_SUCCESS;
        }

        ErrorCode OpenVinoNetWork::OutputBlobsGet(const Blob ***blob_arr, size_t *blob_size)
        {
            if (blob_arr == nullptr || blob_size == nullptr)
            {
                // log todo
                return RS_INVALID_PARAM;
            }

            if (output_blob_arr_ == nullptr || output_blob_size_ <= 0)
            {
                return RS_INVALID_MODEL;
            }

            *blob_arr = (const Blob **)output_blob_arr_;
            *blob_size = output_blob_size_;

            return RS_SUCCESS;
        }

        ErrorCode OpenVinoNetWork::InputBlobGet(const char *input_name, Blob **blob)
        {
            if (input_blob_arr_ == nullptr || blob == nullptr || input_blob_size_ <= 0)
            {
                // log todo
                return RS_INVALID_MODEL;
            }

            int index = -1;
            *blob = FindBlobAndIndexByName(input_blob_arr_, input_blob_size_, input_name, &index);
            if (*blob == NULL)
            {
                // RS_LOGE("Not Find Blob name:%s", input_name != nullptr ? input_name : "");
                return RS_INVALID_PARAM;
            }

            return RS_SUCCESS;
        }

        ErrorCode OpenVinoNetWork::OutputBlobGet(const char *output_name, const Blob **blob)
        {
            if (output_blob_arr_ == nullptr)
            {
                // RS_LOGE("output_blob_arr_ is nullptr");
                return RS_INVALID_MODEL;
            }
            if (output_blob_size_ <= 0)
            {
                // RS_LOGE("output_blob_size_:%d is empty", output_blob_size_);
                return RS_INVALID_MODEL;
            }

            int index = -1; // 这个接口放在工具api中
            Blob *find_blob = FindBlobAndIndexByName(output_blob_arr_, output_blob_size_, output_name, &index);
            if (find_blob == nullptr)
            {
                return RS_INVALID_PARAM;
            }
            //*find_blob = *output_blob_arr_[index];
            // 如果是浅拷贝那就不需要获取输出了 如果是深拷贝那就需要获取输出了,然后

            *blob = find_blob;

            return RS_SUCCESS;
        }

    } // namespace inference
} // namespace rayshape
