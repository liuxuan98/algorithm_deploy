#include "inference/onnxruntime/onnxruntime_network.h"

#include "inference/onnxruntime/onnxruntime_config_converter.h"
#include "model/onnx/onnx_model.h"
#include "utils/memory_size_info.h"
#include "utils/blob_utils.h"
#include "utils/debug_utils.h"
#include "base/logger.h"
#include "utils/device_convert_utils.h"

using namespace rayshape::onnxruntime;
using namespace rayshape::utils;

namespace rayshape
{
    namespace inference
    {
        TypeInferenceRegister<TypeInferenceCreator<ONNXRuntimeNetWork>>
            g_onnxruntime_inference_register(InferenceType::ONNXRUNTIME);

        ONNXRuntimeNetWork::ONNXRuntimeNetWork(InferenceType type) : Inference(type) {}

        ONNXRuntimeNetWork::~ONNXRuntimeNetWork() {
            ClearBlobArray();
        }

        ErrorCode ONNXRuntimeNetWork::Init(const Model *model, const CustomRuntime *runtime) {
            RS_LOGD("ONNXRuntimeNetWork Init!\n");

            if (!model || !runtime) {
                RS_LOGE("model or runtime is null\n");
                return ErrorCode::RS_INVALID_PARAM;
            }

            const ONNXModel *onnx_model = dynamic_cast<const ONNXModel *>(model);
            if (!onnx_model) {
                RS_LOGE("Convert Model to ONNXRuntimeModel failed!\n");
                return ErrorCode::RS_INVALID_PARAM;
            }

            env_ = std::make_shared<Ort::Env>();
            if (!env_) {
                RS_LOGE("Init env failed!\n");
                return ErrorCode::RS_MODEL_ERROR;
            }

            const auto &onnx_data = onnx_model->bin_buf_;
            Ort::SessionOptions options;
#ifdef RS_ONNXRUNTIME_PROVIDER_CUDA
            OrtCUDAProviderOptions cuda_options;
            options.AppendExecutionProvider_CUDA(cuda_options);
#endif
            session_ =
                std::make_shared<Ort::Session>(*env_, onnx_data.data(), onnx_data.size(), options);
            if (!session_) {
                RS_LOGE("Init session failed!\n");
                return ErrorCode::RS_MODEL_ERROR;
            }

            // TODO: 支持动态shape
            // if ((ret = Reshape()) != RS_SUCCESS)
            // {
            //     RS_LOGE("Reshape failed:%d!", ret);
            //     return ret;
            // }

            CHECK_RET(CreateBlobArray())

            for (size_t i = 0; i < input_blob_size_; ++i) {
                Blob *blob = input_blob_arr_[i];
                Buffer *blob_buffer = blob->buffer;
                bool need_set_input = blob_buffer->GetExternalFlag();
                if (!need_set_input) {
                    const char *name = blob->name;

                    void *input_mem = blob->buffer->GetDataPtr();
                    if (input_mem == nullptr) {
                        RS_LOGE("input mem is null\n");
                        return RS_NULL_PARAM;
                    }

                    input_names_.emplace_back(name);

                    auto memory_info =
                        Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
                    std::vector<int64_t> dims;
                    CHECK_RET(ONNXRuntimeConfigConverter::ConvertFromDims(dims, blob->dims));
                    ONNXTensorElementDataType dtype;
                    CHECK_RET(
                        ONNXRuntimeConfigConverter::ConvertFromDataType(dtype, blob->data_type));
                    input_tensors_.emplace_back(Ort::Value::CreateTensor(
                        memory_info, input_mem,
                        blob->buffer->GetDataSize()
                            * GetBytesSize(blob->buffer->GetMemoryInfo().data_type_),
                        dims.data(), dims.size(), dtype));

#if 1
                    RS_LOGD("blob name: %s\n", name);
                    RS_LOGD("blob dtype: %d\n", blob->data_type);
                    RS_LOGD("blob format: %d\n", blob->data_format);
                    RS_LOGD("blob device: %d\n", blob->device_type);
                    RS_LOGD("blob dims: %d, %d, %d, %d\n", blob->dims.value[0], blob->dims.value[1],
                            blob->dims.value[2], blob->dims.value[3]);
                    RS_LOGD("blob bytes: %zu\n",
                            blob->buffer->GetDataSize()
                                * GetBytesSize(blob->buffer->GetMemoryInfo().data_type_));
                    RS_LOGD("blob alloc flag: %d\n", blob->buffer->GetExternalFlag());
#endif
                }
            }

            for (size_t i = 0; i < output_blob_size_; ++i) {
                Blob *blob = output_blob_arr_[i];
                Buffer *blob_buffer = blob->buffer;
                bool need_set_output = blob_buffer->GetExternalFlag();
                if (!need_set_output) {
                    const char *name = blob->name;

                    void *output_mem = blob->buffer->GetDataPtr();
                    if (output_mem == nullptr) {
                        RS_LOGE("output mem is null\n");
                        return RS_NULL_PARAM;
                    }

                    output_names_.emplace_back(name);

                    auto memory_info =
                        Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
                    std::vector<int64_t> dims;
                    CHECK_RET(ONNXRuntimeConfigConverter::ConvertFromDims(dims, blob->dims));
                    ONNXTensorElementDataType dtype;
                    CHECK_RET(
                        ONNXRuntimeConfigConverter::ConvertFromDataType(dtype, blob->data_type));
                    output_tensors_.emplace_back(Ort::Value::CreateTensor(
                        memory_info, output_mem,
                        blob->buffer->GetDataSize()
                            * GetBytesSize(blob->buffer->GetMemoryInfo().data_type_),
                        dims.data(), dims.size(), dtype));

#if 1
                    RS_LOGD("blob name: %s\n", name);
                    RS_LOGD("blob dtype: %d\n", blob->data_type);
                    RS_LOGD("blob format: %d\n", blob->data_format);
                    RS_LOGD("blob device: %d\n", blob->device_type);
                    RS_LOGD("blob dims: %d, %d, %d, %d\n", blob->dims.value[0], blob->dims.value[1],
                            blob->dims.value[2], blob->dims.value[3]);
                    RS_LOGD("blob bytes: %zu\n",
                            blob->buffer->GetDataSize()
                                * GetBytesSize(blob->buffer->GetMemoryInfo().data_type_));
                    RS_LOGD("blob alloc flag: %d\n", blob->buffer->GetExternalFlag());
#endif
                }
            }

            return RS_SUCCESS;
        }

        void ONNXRuntimeNetWork::DeInit() {}

        ErrorCode ONNXRuntimeNetWork::Reshape(const char **name_arr, const Dims *dims_arr,
                                              size_t dims_size) {
            // TODO: Support Dynamic model
            return RS_NOT_IMPLEMENT;
        }

        ErrorCode ONNXRuntimeNetWork::CreateOrUpdateBlob(Blob **dst, const char *blob_name,
                                                         size_t idx, bool is_input) {
            if (dst == nullptr || blob_name == nullptr) {
                RS_LOGE("input params dst or blob_name is null\n");
                return RS_INVALID_PARAM;
            }
            ErrorCode ret = RS_SUCCESS;

            if (*dst != nullptr) {
                free(*dst); // 防止内存泄漏
                *dst = nullptr;
            }

            Blob *blob = (Blob *)malloc(sizeof(Blob));
            if (blob == nullptr) {
                RS_LOGE("blob malloc Blob:%zu failed.\n", sizeof(Blob));
                return RS_OUTOFMEMORY;
            }
            memset(blob, 0, sizeof(Blob));

            Ort::TypeInfo type_info(nullptr);
            if (is_input) {
                type_info = session_->GetInputTypeInfo(idx);
            } else {
                type_info = session_->GetOutputTypeInfo(idx);
            }
            Ort::ConstTensorTypeAndShapeInfo type_and_shape_info =
                type_info.GetTensorTypeAndShapeInfo();
            // convert data type
            blob->data_type =
                ONNXRuntimeConfigConverter::ConvertToDataType(type_and_shape_info.GetElementType());
            // convert shape dims
            ret = ONNXRuntimeConfigConverter::ConvertToDims(blob->dims,
                                                            type_and_shape_info.GetShape());
            if (ret != RS_SUCCESS) {
                RS_LOGE("ONNXRuntimeConfigConverter::ConvertToDims failed:%d.\n", ret);
                ClearBlobArray();
                return ret;
            }

            // convert data layout
            std::string layout = GetDataLayoutString(blob->dims);
            blob->data_format = ONNXRuntimeConfigConverter::ConvertToDataFormat(layout);

            RS_LOGD("blob `%s` device: `CPU`\n", blob_name);
            blob->device_type = DeviceType::CPU;

            // blob name
            strncpy(blob->name, blob_name, strlen(blob_name));

            MemoryType mem_type = MemoryType::NONE;
            ret = ConvertDeviceTypeToMemory(blob->device_type, mem_type);
            if (ret != RS_SUCCESS) {
                RS_LOGE("ConvertDeviceTypeToMemory failed\n");
                delete blob;
                return ret;
            }

            {
                size_t size = CalculateDims(blob->dims);
                RSMemoryInfo mem_info{mem_type, blob->data_type, static_cast<unsigned int>(size)};
                blob->buffer = Buffer::Alloc(mem_info);
                if (blob->buffer == nullptr) {
                    RS_LOGE("Buffer Alloc failed\n");
                    return RS_OUTOFMEMORY;
                }
            }

            *dst = blob;
            return RS_SUCCESS;
        }

        ErrorCode ONNXRuntimeNetWork::CreateBlobArray() {
            ErrorCode ret = RS_SUCCESS;
            ClearBlobArray();

            std::vector<std::string> input_names;
            std::vector<std::string> output_names;
            {
                Ort::AllocatorWithDefaultOptions allocator;

                RS_LOGD("Inputs:\n");
                for (int i = 0; i < session_->GetInputCount(); i++) {
                    auto type_info = session_->GetInputTypeInfo(i);
                    ONNXType type = type_info.GetONNXType();
                    if (type == ONNXType::ONNX_TYPE_TENSOR) {
                        Ort::AllocatedStringPtr name =
                            session_->GetInputNameAllocated(i, allocator);
                        input_names.emplace_back(std::string(name.get()));
                        RS_LOGD("input_name: %s\n", input_names.back().c_str());
                    }
                }

                RS_LOGD("Outputs:\n");
                for (int i = 0; i < session_->GetOutputCount(); i++) {
                    auto type_info = session_->GetOutputTypeInfo(i);
                    ONNXType type = type_info.GetONNXType();
                    if (type == ONNXType::ONNX_TYPE_TENSOR) {
                        Ort::AllocatedStringPtr name =
                            session_->GetOutputNameAllocated(i, allocator);
                        output_names.emplace_back(std::string(name.get()));
                        RS_LOGD("output_name: %s\n", output_names.back().c_str());
                    }
                }
            }

            // 创建输入Blob
            {
                size_t input_count = input_names.size();
                if (input_count <= 0) {
                    RS_LOGE("onnxruntime model inputs count:%zu error.\n", input_count);
                    return RS_INVALID_MODEL;
                }
                input_blob_arr_ = (Blob **)malloc(sizeof(Blob *) * input_count);
                if (input_blob_arr_ == nullptr) {
                    RS_LOGE("input blobs malloc Blob:%zu * size:%zu failed \n", sizeof(Blob),
                            input_count);

                    return RS_OUTOFMEMORY;
                }
                memset(input_blob_arr_, 0, sizeof(Blob *) * input_count);

                for (size_t i = 0; i < input_names.size(); i++) {
                    const std::string &name = input_names.at(i);

                    if (name.empty() || name.size() > MAX_BLOB_NAME) {
                        RS_LOGE("get input name:%s len:%zu failed\n", name.c_str(), name.size());
                        ClearBlobArray(); // 清除分配的内存
                        return RS_INVALID_MODEL;
                    }

                    // 创建Blob，并分配内存
                    ret = CreateOrUpdateBlob(&input_blob_arr_[input_blob_size_++], name.c_str(), i,
                                             true);
                    if (ret != RS_SUCCESS) {
                        RS_LOGE("CreateOrUpdateBlob failed:%d!\n", ret);
                        ClearBlobArray();
                        return ret;
                    }
                }
            }

            // 创建输出Blob
            {
                size_t output_count = output_names.size();
                if (output_count <= 0) {
                    RS_LOGE("onnxruntime model outputs count:%zu error.\n", output_count);
                    ClearBlobArray();
                    return RS_INVALID_MODEL;
                }

                output_blob_arr_ = (Blob **)malloc(sizeof(Blob *) * output_count);
                if (output_blob_arr_ == nullptr) {
                    RS_LOGE("output blobs malloc Blob:%zu * size:%zu failed\n", sizeof(Blob),
                            output_count);
                    ClearBlobArray();
                    return RS_OUTOFMEMORY;
                }
                memset(output_blob_arr_, 0, sizeof(Blob *) * output_count);

                for (size_t i = 0; i < output_names.size(); i++) {
                    const std::string &name = output_names.at(i);

                    if (name.empty() || name.size() > MAX_BLOB_NAME) {
                        RS_LOGE("get output name:%s len:%zu failed\n", name.c_str(), name.size());
                        ClearBlobArray(); // 清除分配的内存
                        return RS_INVALID_MODEL;
                    }

                    // 创建Blob，并分配显存
                    ret = CreateOrUpdateBlob(&output_blob_arr_[output_blob_size_++], name.c_str(),
                                             i, false);
                    if (ret != RS_SUCCESS) {
                        RS_LOGE("CreateOrUpdateBlob failed:%d!\n", ret);
                        ClearBlobArray();
                        return ret;
                    }
                }
            }

            return RS_SUCCESS;
        }

        void ONNXRuntimeNetWork::ClearBlobArray() {
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

        ErrorCode ONNXRuntimeNetWork::Forward() {
            Ort::RunOptions run_options;
            session_->Run(run_options, input_names_.data(), input_tensors_.data(),
                          input_names_.size(), output_names_.data(), output_tensors_.data(),
                          output_names_.size());

            return RS_SUCCESS;
        }

        ErrorCode ONNXRuntimeNetWork::InputBlobsGet(const Blob ***blob_arr, size_t *blob_size) {
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

        ErrorCode ONNXRuntimeNetWork::OutputBlobsGet(const Blob ***blob_arr, size_t *blob_size) {
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

        ErrorCode ONNXRuntimeNetWork::InputBlobGet(const char *input_name, Blob **blob) {
            if (input_blob_arr_ == nullptr || blob == nullptr || input_blob_size_ <= 0) {
                RS_LOGE("blob:%p input_blob_arr_ is nullptr or input_blob_size_:%zu is empty\n",
                        blob, input_blob_size_);
                return RS_INVALID_MODEL;
            }

            int index = -1;
            *blob = FindBlobAndIndexByName(input_blob_arr_, input_blob_size_, input_name, &index);
            if (*blob == NULL) {
                RS_LOGE("Not find Blob name:%s\n", input_name != nullptr ? input_name : "");
                return RS_INVALID_PARAM;
            }

            return RS_SUCCESS;
        }

        ErrorCode ONNXRuntimeNetWork::OutputBlobGet(const char *output_name, const Blob **blob) {
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
