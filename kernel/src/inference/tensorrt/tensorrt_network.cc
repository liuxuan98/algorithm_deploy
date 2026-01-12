#include "inference/tensorrt/tensorrt_network.h"

#include "inference/tensorrt/tensorrt_logger.h"
#include "inference/tensorrt/tensorrt_config_converter.h"
#include "tensorrt_builder.h"
#include "model/onnx/onnx_model.h"
#include "utils/memory_size_info.h"
#include "utils/blob_utils.h"
#include "utils/debug_utils.h"
#include "utils/json_utils.h"
#include "base/logger.h"
#include "utils/device_convert_utils.h"

using namespace rayshape::tensorrt;
using namespace rayshape::utils;

namespace rayshape
{
    namespace inference
    {
        TypeInferenceRegister<TypeInferenceCreator<TensorRTNetWork>> g_tensorrt_inference_register(
            InferenceType::TENSORRT);

        TensorRTNetWork::TensorRTNetWork(InferenceType type) : Inference(type) {}

        TensorRTNetWork::~TensorRTNetWork() {
            ClearBlobArray();
        }

        ErrorCode InitBuilderConfigFromJson(const RSJsonHandle &json_handle,
                                            TensorRTBuilder::Config &config) {
            RSJsonObject root_obj = RSJsonRootGet(json_handle);

            RSJsonObject version_obj = RSJsonObjectGet(root_obj, "version");
            const char *version = RSJsonStringGet(version_obj);
            if (version == nullptr || strlen(version) <= 0 || strlen(version) > MAX_BLOB_NAME) {
                RS_LOGE("version is empty or > MAX_BLOB_NAME:%d!\n", MAX_BLOB_NAME);
                return RS_INVALID_PARAM;
            }
            config.model_version = version;
            RS_LOGD("BuilderConfig version: %s\n", config.model_version.c_str());

            RSJsonObject network_obj = RSJsonObjectGet(root_obj, "NetworkConfig");
            if (network_obj == nullptr) {
                RS_LOGE("key NetworkConfig's value is empty!\n");
                return RS_INVALID_PARAM;
            }
            RSJsonObject builder_config_obj = RSJsonObjectGet(network_obj, "BuilderConfig");
            if (builder_config_obj == nullptr) {
                RS_LOGE("key BuilderConfig's value is empty!\n");
                return RS_INVALID_PARAM;
            }

            RSJsonObject precision_obj = RSJsonObjectGet(builder_config_obj, "Precision");
            const char *precision = RSJsonStringGet(precision_obj);
            if (precision == nullptr || strlen(precision) <= 0
                || strlen(precision) > MAX_BLOB_NAME) {
                RS_LOGE("BuilderConfig Precision is empty or > MAX_BLOB_NAME:%d!\n", MAX_BLOB_NAME);
                return RS_INVALID_PARAM;
            }
            if (std::string(precision) == "FP32") {
                config.precision = TensorRTBuilder::Precision::FP32;
            } else if (std::string(precision) == "FP16") {
                config.precision = TensorRTBuilder::Precision::FP16;
            } else if (std::string(precision) == "INT8") {
                config.precision = TensorRTBuilder::Precision::INT8;
            } else {
                RS_LOGE("Unsupported BuilderConfig Precision: %s\n", precision);
                return RS_INVALID_PARAM;
            }
            RS_LOGD("BuilderConfig precision: %d\n", int(config.precision));

#if NV_TENSORRT_MAJOR > 7
            RSJsonObject profiling_verbo_obj =
                RSJsonObjectGet(builder_config_obj, "ProfilingVerbosity");
            const char *profiling_verbo = RSJsonStringGet(profiling_verbo_obj);
            if (profiling_verbo == nullptr || strlen(profiling_verbo) <= 0
                || strlen(profiling_verbo) > MAX_BLOB_NAME) {
                RS_LOGE("BuilderConfig ProfilingVerbosity is empty or > MAX_BLOB_NAME:%d!\n",
                        MAX_BLOB_NAME);
                return RS_INVALID_PARAM;
            }
            if (std::string(profiling_verbo) == "layer_names_only") {
                config.profiling_verbosity = nvinfer1::ProfilingVerbosity::kLAYER_NAMES_ONLY;
            } else if (std::string(profiling_verbo) == "none") {
                config.profiling_verbosity = nvinfer1::ProfilingVerbosity::kNONE;
            } else if (std::string(profiling_verbo) == "detailed") {
                config.profiling_verbosity = nvinfer1::ProfilingVerbosity::kDETAILED;
            } else {
                RS_LOGE("Unsupported BuilderConfig ProfilingVerbosity: %s\n", profiling_verbo);
                return RS_INVALID_PARAM;
            }
            RS_LOGD("BuilderConfig profiling_verbosity: %d\n", int(config.profiling_verbosity));

            RSJsonObject opt_level_obj = RSJsonObjectGet(builder_config_obj, "OptLevel");
            config.opt_level = RSJsonIntGet(opt_level_obj, 3);
            RS_LOGD("BuilderConfig opt_level: %d\n", config.opt_level);
#endif

            RSJsonObject disable_tf32_obj = RSJsonObjectGet(builder_config_obj, "DisableTF32");
            config.disable_tf32 = RSJsonBoolGet(disable_tf32_obj, false);
            RS_LOGD("BuilderConfig disable_tf32: %s\n", (config.disable_tf32 ? "true" : "false"));

            RSJsonObject use_dla_core_obj = RSJsonObjectGet(builder_config_obj, "UseDLACore");
            config.use_dla_core = RSJsonIntGet(use_dla_core_obj, 3);
            RS_LOGD("BuilderConfig use_dla_core: %d\n", config.use_dla_core);

            RSJsonObject engine_fname_obj = RSJsonObjectGet(builder_config_obj, "EngineFileName");
            const char *engine_fname = RSJsonStringGet(engine_fname_obj);
            if (engine_fname != nullptr && strlen(engine_fname) > MAX_BLOB_NAME) {
                RS_LOGE("BuilderConfig EngineFileName is > MAX_BLOB_NAME:%d!\n", MAX_BLOB_NAME);
                return RS_INVALID_PARAM;
            }
            config.engine_fname = engine_fname;
            RS_LOGD("BuilderConfig engine_fname: %s\n", config.engine_fname.c_str());

#if NV_TENSORRT_MAJOR > 7
            RSJsonObject timing_cache_fname_obj =
                RSJsonObjectGet(builder_config_obj, "TimingCacheFileName");
            const char *timing_cache_fname = RSJsonStringGet(timing_cache_fname_obj);
            if (timing_cache_fname != nullptr && strlen(timing_cache_fname) > MAX_BLOB_NAME) {
                RS_LOGE("BuilderConfig TimingCacheFileName is empty or > MAX_BLOB_NAME:%d!\n",
                        MAX_BLOB_NAME);
                return RS_INVALID_PARAM;
            }
            config.timing_cache_fname = timing_cache_fname;
            RS_LOGD("BuilderConfig timing_cache_fname: %s\n", config.timing_cache_fname.c_str());
#endif

            RSJsonObject cache_dir_obj = RSJsonObjectGet(builder_config_obj, "CachePath");
            const char *cache_dir = RSJsonStringGet(cache_dir_obj);
            if (cache_dir == nullptr || strlen(cache_dir) <= 0
                || strlen(cache_dir) > MAX_BLOB_NAME) {
                RS_LOGE("BuilderConfig CachePath is empty or > MAX_BLOB_NAME:%d!\n", MAX_BLOB_NAME);
                return RS_INVALID_PARAM;
            }
            config.cache_dir = cache_dir;
            RS_LOGD("BuilderConfig cache_dir: %s\n", config.cache_dir.c_str());

            return ErrorCode::RS_SUCCESS;
        }

        ErrorCode TensorRTNetWork::Init(const Model *model, const CustomRuntime *runtime) {
            RS_LOGD("TensorRTNetWork Init!\n");

            if (!model || !runtime) {
                RS_LOGE("model or runtime is null\n");
                return ErrorCode::RS_INVALID_PARAM;
            }

            runtime_.reset(nvinfer1::createInferRuntime(tensorrt::g_logger));

            ModelType model_type = model->GetModelType();
            switch (model_type) {
            case ModelType::ONNX: {
                const ONNXModel *onnx_model = dynamic_cast<const ONNXModel *>(model);
                if (!onnx_model) {
                    RS_LOGE("Convert Model to ONNXRuntimeModel failed!\n");
                    return ErrorCode::RS_INVALID_PARAM;
                }
                const auto &onnx_data = onnx_model->bin_buf_;

                RSJsonHandle json_handle = nullptr;
                ErrorCode err;
                if ((err = RSJsonCreate(onnx_model->cfg_str_.c_str(), &json_handle))
                    != RS_SUCCESS) {
                    RS_LOGE("RSJsonCreate failed:%d\n", err);
                    return err;
                }

                std::vector<char> engine_data;
                TensorRTBuilder::Config builder_config;
                if ((err = InitBuilderConfigFromJson(json_handle, builder_config)) != RS_SUCCESS) {
                    RS_LOGE("Init builder config failed:%d\n", err);
                    return err;
                }
                if ((err = TensorRTBuilder::Build(onnx_data, builder_config, engine_data))
                    != RS_SUCCESS) {
                    RS_LOGE("Build engine failed:%d\n", err);
                    return err;
                }

                engine_.reset(
                    runtime_->deserializeCudaEngine(engine_data.data(), engine_data.size()));
                if (!engine_) {
                    RS_LOGE("Init engine failed\n");
                    return ErrorCode::RS_MODEL_ERROR;
                }

                break;
            }
            case ModelType::TENSORRT: {
                // TODO: Support TensorRT Model

                // const auto &engine_data = trt_model->GetEngineData();
                // engine_.reset(runtime_->deserializeCudaEngine(engine_data.data(),
                // engine_data.size())); if (!engine_) {
                //     RS_LOGE("Init engine failed!\n");
                //     return ErrorCode::RS_MODEL_ERROR;
                // }
                return ErrorCode::RS_NOT_IMPLEMENT;
                break;
            }
            default: {
                RS_LOGE("Unsupported model type: %d\n", int(model_type));
                return ErrorCode::RS_INVALID_PARAM;
            }
            }

            // TODO: 支持动态shape
            // if ((ret = Reshape()) != RS_SUCCESS)
            // {
            //     RS_LOGE("Reshape failed:%d!", ret);
            //     return ret;
            // }

            CHECK_RET(CreateBlobArray())

            exec_ctx_ = tensorrt::TrtUniquePtr<nvinfer1::IExecutionContext>(
                engine_->createExecutionContext());
            if (!exec_ctx_) {
                RS_LOGE("Init ctx failed!\n");
                return RS_INVALID_PARAM;
            };

            for (size_t i = 0; i < input_blob_size_; ++i) {
                Blob *blob = input_blob_arr_[i];
                Buffer *blob_buffer = blob->buffer;
                bool need_set_input = blob_buffer->GetExternalFlag();
                if (!need_set_input) {
                    const char *name = blob->name;

                    nvinfer1::Dims dims;
                    CHECK_RET(TensorRTConfigConverter::ConvertFromDims(dims, blob->dims));
#if NV_TENSORRT_MAJOR > 7
                    exec_ctx_->setInputShape(name, dims);
#else
                    int binding_idx = engine_->getBindingIndex(name);
                    exec_ctx_->setBindingDimensions(binding_idx, dims);
#endif

                    void *input_mem = blob->buffer->GetDataPtr();
                    if (input_mem == nullptr) {
                        RS_LOGE("input mem is null\n");
                        return RS_INVALID_PARAM;
                    }
#if NV_TENSORRT_MAJOR > 7
                    exec_ctx_->setTensorAddress(name, input_mem);
#else
                    bindings_.emplace_back(input_mem);
#endif

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
#if NV_TENSORRT_MAJOR > 7
                    RS_LOGD("trt format: %s\n", engine_->getTensorFormatDesc(name));
#endif
#endif
                }
            }

            for (size_t i = 0; i < output_blob_size_; ++i) {
                Blob *blob = output_blob_arr_[i];
                Buffer *blob_buffer = blob->buffer;
                bool need_set_output = blob_buffer->GetExternalFlag();
                if (!need_set_output) {
                    const char *name = blob->name;

#if 0
                        // check dims
                        nvinfer1::Dims64 dims;
                        CHECK_RET(TensorRTConfigConverter::ConvertFromDims(dims, blob->dims));
                        auto output_dims = exec_ctx_->getTensorShape(name);
                        assert(dims.nbDims == output_dims.nbDims);
                        for(int i = 0; i < dims.nbDims; i++) {
                            assert(dims.d[i] == output_dims.d[i]);
                        }
#endif

                    void *output_mem = blob->buffer->GetDataPtr();
                    if (output_mem == nullptr) {
                        RS_LOGE("output mem is null\n");
                        return RS_INVALID_PARAM;
                    }
#if NV_TENSORRT_MAJOR > 7
                    exec_ctx_->setTensorAddress(name, output_mem);
#else
                    bindings_.emplace_back(output_mem);
#endif

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
#if NV_TENSORRT_MAJOR > 7
                    RS_LOGD("trt format: %s\n", engine_->getTensorFormatDesc(name));
#endif
#endif
                }
            }

            return RS_SUCCESS;
        }

        void TensorRTNetWork::DeInit() {}

        ErrorCode TensorRTNetWork::Reshape(const char **name_arr, const Dims *dims_arr,
                                           size_t dims_size) {
            // TODO: Support Dynamic model
            return RS_NOT_IMPLEMENT;
        }

        ErrorCode TensorRTNetWork::CreateOrUpdateBlob(Blob **dst, const char *blob_name) {
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

#if NV_TENSORRT_MAJOR > 7
            // convert data type
            blob->data_type =
                TensorRTConfigConverter::ConvertToDataType(engine_->getTensorDataType(blob_name));
            // convert shape dims
            ret = TensorRTConfigConverter::ConvertToDims(blob->dims,
                                                         engine_->getTensorShape(blob_name));
#else
            int binding_idx = engine_->getBindingIndex(blob_name);
            // convert data type
            blob->data_type = TensorRTConfigConverter::ConvertToDataType(
                engine_->getBindingDataType(binding_idx));
            // convert shape dims
            ret = TensorRTConfigConverter::ConvertToDims(
                blob->dims, engine_->getBindingDimensions(binding_idx));
#endif
            if (ret != RS_SUCCESS) {
                RS_LOGE("TensorRTConfigConverter::ConvertToDims failed:%d.\n", ret);
                delete blob;
                return ret;
            }

            // convert data layout
            std::string layout = GetDataLayoutString(blob->dims);
            blob->data_format = TensorRTConfigConverter::ConvertToDataFormat(layout);

#if NV_TENSORRT_MAJOR > 7
            nvinfer1::TensorLocation loc = engine_->getTensorLocation(blob_name);
#else
            nvinfer1::TensorLocation loc = engine_->getLocation(binding_idx);
#endif
            switch (loc) {
            case nvinfer1::TensorLocation::kHOST:
                RS_LOGD("blob `%s` device: `CPU`\n", blob_name);
                blob->device_type = DeviceType::CPU;
                break;
            case nvinfer1::TensorLocation::kDEVICE:
                RS_LOGD("blob `%s` device: `CUDA`\n", blob_name);
                blob->device_type = DeviceType::CUDA;
                break;
            }
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

        ErrorCode TensorRTNetWork::CreateBlobArray() {
            ErrorCode ret = RS_SUCCESS;
            ClearBlobArray();

            std::vector<const char *> input_names;
            std::vector<const char *> output_names;
#if NV_TENSORRT_MAJOR > 7
            for (int i = 0; i < engine_->getNbIOTensors(); i++) {
                const char *name = engine_->getIOTensorName(i);
                nvinfer1::TensorIOMode io_mode = engine_->getTensorIOMode(name);
                switch (io_mode) {
                case nvinfer1::TensorIOMode::kINPUT: {
                    RS_LOGD("input name: %s\n", name);
                    input_names.emplace_back(name);
                    break;
                }
                case nvinfer1::TensorIOMode::kOUTPUT: {
                    RS_LOGD("output name: %s\n", name);
                    output_names.emplace_back(name);
                    break;
                }
                }
            }
#else
            for (int i = 0; i < engine_->getNbBindings(); i++) {
                const char *name = engine_->getBindingName(i);
                if (engine_->bindingIsInput(i)) {
                    RS_LOGD("input name: %s\n", name);
                    input_names.emplace_back(name);
                } else {
                    RS_LOGD("output name: %s\n", name);
                    output_names.emplace_back(name);
                }
            }
#endif

            // 创建输入Blob
            {
                size_t input_count = input_names.size();
                if (input_count <= 0) {
                    RS_LOGE("tensorrt model inputs count:%zu error.\n", input_count);
                    return RS_INVALID_MODEL;
                }
                input_blob_arr_ = (Blob **)malloc(sizeof(Blob *) * input_count);
                if (input_blob_arr_ == nullptr) {
                    RS_LOGE("input blobs malloc Blob:%zu * size:%zu failed \n", sizeof(Blob),
                            input_count);

                    return RS_OUTOFMEMORY;
                }
                memset(input_blob_arr_, 0, sizeof(Blob *) * input_count);

                for (auto name : input_names) {
                    if (name == NULL || strlen(name) <= 0 || strlen(name) > MAX_BLOB_NAME) {
                        RS_LOGE("get input name:%s len:%zu failed\n", name != NULL ? name : "",
                                name != NULL ? strlen(name) : 0);
                        ClearBlobArray(); // 清除分配的内存
                        return RS_INVALID_MODEL;
                    }

                    // 创建Blob，并分配显存
                    ret = CreateOrUpdateBlob(&input_blob_arr_[input_blob_size_++], name);
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
                    RS_LOGE("tensorrt model outputs count:%zu error.\n", output_count);
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

                for (const auto name : output_names) {
                    if (name == NULL || strlen(name) <= 0 || strlen(name) > MAX_BLOB_NAME) {
                        RS_LOGE("get output name:%s len:%zu failed\n", name != NULL ? name : "",
                                name != NULL ? strlen(name) : 0);
                        ClearBlobArray(); // 清除分配的内存
                        return RS_INVALID_MODEL;
                    }

                    // 创建Blob，并分配显存
                    ret = CreateOrUpdateBlob(&output_blob_arr_[output_blob_size_++], name);
                    if (ret != RS_SUCCESS) {
                        RS_LOGE("CreateOrUpdateBlob failed:%d!\n", ret);
                        ClearBlobArray();
                        return ret;
                    }
                }
            }

            return ret;
        }

        void TensorRTNetWork::ClearBlobArray() {
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

        ErrorCode TensorRTNetWork::Forward() {
            {
                cudaStream_t stream;
                if (cudaStreamCreate(&stream) != cudaSuccess) {
                    RS_LOGE("cuda stream creation failed\n");
                    return RS_MODEL_ERROR;
                }

// Run TensorRT inference
#if NV_TENSORRT_MAJOR > 7
                bool status = exec_ctx_->enqueueV3(stream);
#else
                bool status = exec_ctx_->enqueueV2(bindings_.data(), stream, nullptr);
#endif
                if (!status) {
                    RS_LOGE("TensorRT inference failed\n");
                    return RS_MODEL_ERROR;
                }
                cudaStreamSynchronize(stream);

                cudaStreamDestroy(stream);
            }

            return RS_SUCCESS;
        }

        ErrorCode TensorRTNetWork::InputBlobsGet(const Blob ***blob_arr, size_t *blob_size) {
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

        ErrorCode TensorRTNetWork::OutputBlobsGet(const Blob ***blob_arr, size_t *blob_size) {
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

        ErrorCode TensorRTNetWork::InputBlobGet(const char *input_name, Blob **blob) {
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

        ErrorCode TensorRTNetWork::OutputBlobGet(const char *output_name, const Blob **blob) {
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
