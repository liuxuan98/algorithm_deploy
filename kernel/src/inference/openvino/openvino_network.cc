#include "inference/openvino/openvino_network.h"

#include "inference/openvino/openvino_blob_converter.h"
#include "inference/openvino/openvino_config_converter.h"
#include "model/openvino/openvino_model.h"
#include "utils/blob_utils.h"
#include "base/logger.h"
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <memory>
#include <cstring>
#include <exception>

using namespace rayshape::utils;
using namespace rayshape::inference::openvino;

namespace rayshape
{
    namespace inference
    {
        TypeInferenceRegister<TypeInferenceCreator<OpenVinoNetWork>> g_openvino_inference_register(
            InferenceType::OPENVINO);

        OpenVinoNetWork::OpenVinoNetWork(InferenceType type) : Inference(type) {}

        OpenVinoNetWork::~OpenVinoNetWork() {
            ClearBlobArray();
        }

        ErrorCode OpenVinoNetWork::Init(const Model *model, const CustomRuntime *runtime) {
            RS_LOGD("OpenVinoNetWork Init with Model!\n");

            ErrorCode ret = RS_SUCCESS;

            if (model == nullptr || runtime == nullptr) {
                RS_LOGE("model or runtime is nullptr!\n");
                return RS_INVALID_PARAM;
            }

            device_type_ = runtime->device_type_;
            if (device_type_ == DeviceType::INTERL_GPU) {
                device_name_ = "GPU";
            }
            num_threads_ = runtime->num_thread_;

            auto openvino_model = dynamic_cast<const OpenVINOModel *>(model);
            if (openvino_model == nullptr) {
                RS_LOGE("OpenVINOModel is nullptr!\n");
                return RS_INVALID_MODEL;
            }

            RSJsonHandle json_handle = nullptr;
            do {
                // Parse configuration from the packed model's cfg_str_ (JSON configuration)
                if ((ret = RSJsonCreate(openvino_model->cfg_str_.c_str(), &json_handle))
                    != RS_SUCCESS) {
                    RS_LOGE("RSJsonCreate failed:%d\n", ret);
                    break;
                }

                if ((ret = ParseInputShapes(json_handle)) != RS_SUCCESS) {
                    RS_LOGE("ParseInputShapes failed:%d!\n", ret);
                    break;
                }

                RSJsonObject root_obj = RSJsonRootGet(json_handle);
                RSJsonObject network_config_obj = RSJsonObjectGet(root_obj, "NetworkConfig");
                RSJsonObject cache_dir_obj = RSJsonObjectGet(network_config_obj, "CachePath");
                const char *cache_dir = RSJsonStringGet(cache_dir_obj);
                if (cache_dir == nullptr || strlen(cache_dir) <= 0
                    || strlen(cache_dir) > MAX_BLOB_NAME) {
                    RS_LOGE("BuilderConfig CachePath is empty or > MAX_BLOB_NAME:%d!\n",
                            MAX_BLOB_NAME);
                    return RS_INVALID_PARAM;
                }
                if ((ret = InitWithMemoryContent(openvino_model->xml_content_,
                                                 openvino_model->bin_content_, cache_dir))
                    != RS_SUCCESS) {
                    RS_LOGE("InitWithMemoryContent failed:%d!\n", ret);
                    break;
                }

                if ((ret = Reshape()) != RS_SUCCESS) {
                    RS_LOGE("Reshape failed:%d!\n", ret);
                    break;
                }

                if ((ret = CreateBlobArray()) != RS_SUCCESS) {
                    RS_LOGE("CreateBlobArray failed:%d!\n", ret);
                    break;
                }
            } while (false);

            // Clean up resources
            if (json_handle != nullptr) {
                RSJsonDestory(&json_handle);
            }

            return ret;
        }

        void OpenVinoNetWork::DeInit() {}

        ErrorCode OpenVinoNetWork::InitWithMemoryContent(const std::string &xml_content,
                                                         const std::string &bin_content,
                                                         const std::string &cache_dir) {
            ErrorCode ret = RS_SUCCESS;

            RS_LOGD("OpenVINO model initialization from memory content\n");

            try {
                // Initialize OpenVINO Core if not already done
                if (core_ == nullptr) {
                    core_ = std::make_unique<ov::Core>();
                    core_->set_property(ov::cache_dir(cache_dir));
                }

                // Create a tensor from binary content
                ov::Tensor weights_tensor;
                if (!bin_content.empty()) {
                    weights_tensor = ov::Tensor(ov::element::u8, {bin_content.size()});
                    std::memcpy(weights_tensor.data<uint8_t>(), bin_content.data(),
                                bin_content.size());
                }

                // Load model from memory
                if (weights_tensor) {
                    model_ = core_->read_model(xml_content, weights_tensor);
                } else {
                    model_ = core_->read_model(xml_content);
                }

                if (!model_) {
                    RS_LOGE("Failed to load OpenVINO model from memory content\n");
                    return RS_MODEL_ERROR;
                }

                RS_LOGD("OpenVINO model loaded successfully from memory\n");

            } catch (const ov::Exception &e) {
                RS_LOGE("OpenVINO exception during model loading: %s\n", e.what());
                return RS_MODEL_ERROR;
            } catch (const std::exception &e) {
                RS_LOGE("Standard exception during model loading: %s\n", e.what());
                return RS_MODEL_ERROR;
            }

            return ret;
        }

        ErrorCode OpenVinoNetWork::ParseInputShapes(const RSJsonHandle json_handle) {
            ErrorCode ret = RS_SUCCESS;

            RSJsonObject root_obj = RSJsonRootGet(json_handle);
            /*if (root_obj.HasMember("ModelConfig")) {
                RSJsonObject tmp_obj = root_obj["ModelConfig"].GetObject();
                if (tmp_obj.HasMember("IsDynamic")) {
                    bool tmp_obj_01 = tmp_obj["IsDynamic"].GetBool();
                    RSJsonObject tmp_obj_0 = tmp_obj["IsDynamic"].GetObject();
                    if (tmp_obj_0.IsBool()) {

                        bool t_b = true;
                        t_b = tmp_obj_0.GetBool();

                        std::cout << "is dynamic" << t_b << std::endl;
                    }
                }
                if (tmp_obj.HasMember("MaxShapes")) {
                    RSJsonObject tmp_obj_0 = tmp_obj["MaxShapes"].GetObject();
                    if (tmp_obj_0.IsArray()) {

                      unsigned int tmp = tmp_obj_0.GetArray().Size();
                      std::cout << tmp << std::endl;
                    }
                    std::cout << "is MaxShapes" << std::endl;
                }
            }*/
            RSJsonObject model_obj = RSJsonObjectGet(root_obj, "ModelConfig");
            if (model_obj == nullptr) {
                RS_LOGE("key ModelConfig's value is empty!\n");
                return RS_INVALID_PARAM;
            }
            RSJsonObject max_shapes_arr = RSJsonObjectGet(model_obj, "MaxShapes");
            if (max_shapes_arr == nullptr) {
                return RS_SUCCESS;
            }

            //
            unsigned int size = RSJsonArraySize(max_shapes_arr);
            for (unsigned int i = 0; i < size; i++) {
                RSJsonObject shape_obj = RSJsonArrayAt(max_shapes_arr, i);
                RSJsonObject name_obj = RSJsonObjectGet(shape_obj, "name");
                const char *name = RSJsonStringGet(name_obj);
                if (name == nullptr || strlen(name) <= 0 || strlen(name) > MAX_BLOB_NAME) {
                    RS_LOGE("InputMaxShape Name is empty or > MAX_DIMS_NAME:%d!\n", MAX_BLOB_NAME);
                    return RS_INVALID_MODEL;
                }
                // 获取维度对象
                RSJsonObject dims_arr = RSJsonObjectGet(shape_obj, "dims");
                if (dims_arr == nullptr) {
                    RS_LOGE("InputMaxShape Dims is Error!\n");
                    return RS_INVALID_MODEL;
                }
                // 获取维度的size
                unsigned int dims_size = RSJsonArraySize(dims_arr);
                if (dims_size <= 0 || dims_size > MAX_DIMS_SIZE) {
                    RS_LOGE("InputMaxShape Dims size:%d is empty or > MAX_DIMS_SIZE:%d!\n",
                            dims_size, MAX_DIMS_SIZE);
                    return RS_INVALID_MODEL;
                }

                Dims dims;
                dims.size = dims_size;

                for (unsigned int j = 0; j < dims_size; j++) {
                    dims.value[j] = RSJsonIntGet(RSJsonArrayAt(dims_arr, j), 0);
                }
                input_max_shapes_[name] = dims;
            }

            return RS_SUCCESS;
        }

        ErrorCode OpenVinoNetWork::Reshape(const char **name_arr, const Dims *dims_arr,
                                           size_t dims_size) {
            ErrorCode ret = RS_SUCCESS;

            if (name_arr == nullptr || dims_arr == nullptr) {
                RS_LOGE("Invalid input parameters for Reshape.\n");
                return RS_INVALID_PARAM;
            }
            // dynamic_model need to reshape
            bool reshape_flag = false;
            // std::map<std::string, ov::PartialShape> ov_shape_w;
            std::map<ov::Output<ov::Node>, ov::PartialShape> ov_shape_map;
            for (int i = 0; i < dims_size; ++i) {
                const char *name = name_arr[i];
                const Dims *dims = &dims_arr[i];
                bool tensor_flag = false;

                if (name == nullptr) {
                    RS_LOGE("Input name at index %d is null.\n", i);
                    return RS_INVALID_PARAM;
                }

                ov::Output<ov::Node> input = model_->input(name); // name must be valid
                const ov::Shape &ov_origin_shape = input.get_shape();
                if (ov_origin_shape.size() != dims->size) {
                    tensor_flag = true;
                } else {
                    for (int i = 0; i < dims->size; ++i) {
                        if (dims->value[i] != ov_origin_shape[i]) {
                            tensor_flag = true; // 尺寸没对上
                            break;
                        }
                    }
                }

                // get input tensor

                if (tensor_flag) {
                    std::vector<ov::Dimension> ov_dims;
                    for (size_t j = 0; j < dims->size; ++j) {
                        ov_dims.emplace_back(dims->value[j]);
                    }

                    // ov_shape_w[name] = ov::PartialShape(ov_dims);
                    ov_shape_map[input] = ov::PartialShape(ov_dims);
                    reshape_flag = true;
                }
            }
            // model_->input()
            if (reshape_flag) {
                try {
                    // 重新编译模型
                    model_->reshape(ov_shape_map);
                    // 设置新的形状
                    compiled_model_ = core_->compile_model(
                        model_, device_name_); // 可根据 device_type_ 动态选择设备
                    infer_request_ = compiled_model_.create_infer_request();
                    CreateBlobArray();
                } catch (const ov::Exception &e) {
                    RS_LOGE("Failed to recompile model after reshape: %s.\n", e.what());
                    return RS_MODEL_ERROR;
                }
            }

            return ret;
        }
        // 写入 resize
        ErrorCode OpenVinoNetWork::Reshape() {
            ErrorCode ret = RS_SUCCESS;
            bool is_dynamic_model = model_->is_dynamic();
            if (is_dynamic_model && input_max_shapes_.size() > 0) {
                std::map<std::string, ov::PartialShape> ov_shape;
                for (const auto &it : input_max_shapes_) {
                    const std::string name = it.first;

                    const Dims &dims = it.second;

                    std::vector<ov::Dimension> ov_dims;
                    for (size_t i = 0; i < dims.size; i++) {
                        ov_dims.emplace_back(dims.value[i]);
                    }
                    // 转格式
                    ov_shape[name] = ov::PartialShape(ov_dims);
                }
                model_->reshape(ov_shape);
            }
            // 完成多个最大输入的reshape
            try {
                compiled_model_ = core_->compile_model(model_, device_name_); // 是否要reset
                infer_request_ = compiled_model_.create_infer_request();
            } catch (const ov::Exception &e) {
                RS_LOGE("compile openvino model failed: %s\n", e.what());
                return RS_MODEL_ERROR;
            }
            return ret;
        }

        ErrorCode OpenVinoNetWork::CreateBlobArray() {
            ErrorCode ret = RS_SUCCESS;
            ClearBlobArray();

            const std::vector<ov::Output<ov::Node>> &inputs = model_->inputs();
            size_t input_count = inputs.size();
            if (input_count <= 0) {
                RS_LOGE("openvino model inputs count:%zu error.\n", input_count);
                return RS_INVALID_MODEL;
            }
            input_blob_arr_ = (Blob **)malloc(sizeof(Blob *) * input_count);
            if (input_blob_arr_ == nullptr) {
                RS_LOGE("input blobs malloc Blob:%d * size:%zu failed \n", (int)sizeof(Blob),
                        input_count);
                return RS_OUTOFMEMORY;
            }
            memset(input_blob_arr_, 0, sizeof(Blob *) * input_count);

            for (const auto input : inputs) {
                std::string name_str = input.get_any_name();
                const char *name = name_str.c_str();
                if (name == nullptr || strlen(name) <= 0 || strlen(name) > MAX_BLOB_NAME) {
                    RS_LOGE("get input name:%s len:%zd failed\n", name != nullptr ? name : "",
                            name != NULL ? strlen(name) : 0);
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
                ret = OpenvinoBlobConverter::CreateOrUpdateBlob(
                    &input_blob_arr_[input_blob_size_++], ov_tensor, name, false);
                if (ret != RS_SUCCESS) {
                    RS_LOGE("OpenvinoBlobConverter::CreateOrUpdateBlob failed:%d!\n", ret);
                    ClearBlobArray();
                    return ret;
                }
            }

            const std::vector<ov::Output<ov::Node>> &outputs = model_->outputs();
            size_t output_count = outputs.size();
            if (output_count <= 0) {
                RS_LOGE("openvino model outputs count:%zu error.\n", output_count);
                ClearBlobArray();
                return RS_INVALID_MODEL;
            }

            output_blob_arr_ = (Blob **)malloc(sizeof(Blob *) * output_count);
            if (output_blob_arr_ == nullptr) {
                RS_LOGE("output blobs malloc Blob:%zd * size:%zu failed\n", sizeof(Blob),
                        output_count);
                ClearBlobArray();
                return RS_OUTOFMEMORY;
            }
            memset(output_blob_arr_, 0, sizeof(Blob *) * output_count);
            for (const auto output : outputs) {
                std::string output_name = output.get_any_name();
                const char *name = output_name.c_str();
                if (name == nullptr || strlen(name) <= 0 || strlen(name) > MAX_BLOB_NAME) {
                    RS_LOGE("get output name:%s len:%zd failed", name != nullptr ? name : "",
                            name != nullptr ? strlen(name) : 0);
                    ClearBlobArray();
                    return RS_INVALID_MODEL;
                }
                ov::Tensor ov_tensor = infer_request_.get_tensor(output_name);
                ret = OpenvinoBlobConverter::CreateOrUpdateBlob(
                    &output_blob_arr_[output_blob_size_++], ov_tensor, name,
                    false); // 需不需要分配内存根据实际情况考虑
                if (ret != RS_SUCCESS) {
                    RS_LOGE("OpenvinoBlobConverter::CreateOrUpdateBlob failed:%d!\n", ret);
                    ClearBlobArray();
                    return ret;
                }
            }

            return ret;
        }

        void OpenVinoNetWork::ClearBlobArray() {
            if (input_blob_arr_ != nullptr && input_blob_size_ != 0) {
                for (size_t i = 0; i < input_blob_size_; ++i) {
                    if (input_blob_arr_[i]) {
                        BlobFree(input_blob_arr_[i]);
                    }
                }
                free(input_blob_arr_);
                input_blob_arr_ = nullptr;
                input_blob_size_ = 0;
            }

            if (output_blob_arr_ != nullptr && output_blob_size_ != 0) {
                for (size_t i = 0; i < output_blob_size_; ++i) {
                    if (output_blob_arr_[i]) {
                        BlobFree(output_blob_arr_[i]);
                    }
                }
                free(output_blob_arr_);
                output_blob_arr_ = nullptr;
                output_blob_size_ = 0;
            }
        }

        ErrorCode OpenVinoNetWork::Forward() {
            ErrorCode ret = RS_SUCCESS;
            try {
                for (size_t i = 0; i < input_blob_size_; ++i) {
                    Blob *blob = input_blob_arr_[i];
                    Buffer *blob_buffer = blob->buffer;
                    bool need_set_input = blob_buffer->GetExternalFlag();
                    if (!need_set_input) {
                        const char *name = blob->name;
                        std::shared_ptr<ov::Tensor> ov_tensor =
                            OpenvinoBlobConverter::ConvertFromBlob(ret, blob);
                        if (ov_tensor == nullptr || ret != RS_SUCCESS) {
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
            } catch (const ov::Exception &e) {
                printf("openvino model infer failed: %s\n", e.what());
                return RS_MODEL_ERROR;
            }

            return ret;
        }

        ErrorCode OpenVinoNetWork::InputBlobsGet(const Blob ***blob_arr, size_t *blob_size) {
            if (blob_arr == nullptr || blob_size == nullptr) {
                RS_LOGE("blob_arr:%p or blob_size:%p is nullptr\n", blob_arr, blob_size);
                return RS_INVALID_PARAM;
            }

            if (input_blob_arr_ == nullptr || input_blob_size_ <= 0) {
                RS_LOGE("input_blob_arr_:%p is nullptr or input_blob_size_:%zu <= 0\n",
                        input_blob_arr_, input_blob_size_);
                return RS_INVALID_MODEL;
            }

            *blob_arr = (const Blob **)input_blob_arr_;
            *blob_size = input_blob_size_;

            return RS_SUCCESS;
        }

        ErrorCode OpenVinoNetWork::OutputBlobsGet(const Blob ***blob_arr, size_t *blob_size) {
            if (blob_arr == nullptr || blob_size == nullptr) {
                RS_LOGE("blob_arr:%p or blob_size:%p is nullptr\n", blob_arr, blob_size);
                return RS_INVALID_PARAM;
            }

            if (output_blob_arr_ == nullptr || output_blob_size_ <= 0) {
                RS_LOGE("output_blob_arr_:%p is nullptr or output_blob_size_:%zu <= 0\n",
                        output_blob_arr_, output_blob_size_);
                return RS_INVALID_MODEL;
            }

            *blob_arr = (const Blob **)output_blob_arr_;
            *blob_size = output_blob_size_;

            return RS_SUCCESS;
        }

        ErrorCode OpenVinoNetWork::InputBlobGet(const char *input_name, Blob **blob) {
            if (input_blob_arr_ == nullptr || blob == nullptr || input_blob_size_ <= 0) {
                RS_LOGE("blob:%p input_blob_arr_ is nullptr or input_blob_size_:%zu is empty\n",
                        blob, input_blob_size_);
                return RS_INVALID_MODEL;
            }

            int index = -1;
            *blob = FindBlobAndIndexByName(input_blob_arr_, input_blob_size_, input_name, &index);
            if (*blob == nullptr) {
                RS_LOGE("Not find Blob name:%s\n", input_name != nullptr ? input_name : "");
                return RS_INVALID_PARAM;
            }

            return RS_SUCCESS;
        }

        ErrorCode OpenVinoNetWork::OutputBlobGet(const char *output_name, const Blob **blob) {
            if (output_blob_arr_ == nullptr || blob == nullptr || output_blob_size_ <= 0) {
                RS_LOGE("blob:%p output_blob_arr_ is nullptr or output_blob_size_:%zu is empty\n",
                        blob, output_blob_size_);
                return RS_INVALID_MODEL;
            }

            int index = -1; // 这个接口放在工具api中
            Blob *find_blob =
                FindBlobAndIndexByName(output_blob_arr_, output_blob_size_, output_name, &index);
            if (find_blob == nullptr) {
                RS_LOGE("Not find Blob name:%s\n", output_name != nullptr ? output_name : "");
                return RS_INVALID_PARAM;
            }
            //*find_blob = *output_blob_arr_[index];
            // 如果是浅拷贝那就不需要获取输出了 如果是深拷贝那就需要获取输出了,然后

            *blob = find_blob;

            return RS_SUCCESS;
        }

    } // namespace inference
} // namespace rayshape
