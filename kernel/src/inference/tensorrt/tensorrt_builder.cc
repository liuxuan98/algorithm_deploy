#include "tensorrt_builder.h"

#include <NvOnnxParser.h>

#include "base/logger.h"
#include "utils/kernel_file_utils.h"
#include "inference/tensorrt/tensorrt_logger.h"
#include "inference/tensorrt/tensorrt_common.h"

#if NV_TENSORRT_MAJOR > 7
#include "inference/tensorrt/tensorrt_timing_cache.h"
#endif

namespace rayshape
{
    inline void EnableDLA(nvinfer1::IBuilder *builder, nvinfer1::IBuilderConfig *config,
                          int use_dla_core, bool allow_gpu_fallback = true) {
        if (use_dla_core >= 0) {
            if (builder->getNbDLACores() == 0) {
                RS_LOGW("Trying to use DLA core %d on a platform that doesn't have any DLA cores\n",
                        use_dla_core);
            }
            if (allow_gpu_fallback) {
                config->setFlag(nvinfer1::BuilderFlag::kGPU_FALLBACK);
            }
            if (!config->getFlag(nvinfer1::BuilderFlag::kINT8)) {
                // User has not requested INT8 Mode.
                // By default run in FP16 mode. FP32 mode is not permitted.
                config->setFlag(nvinfer1::BuilderFlag::kFP16);
            }
            config->setDefaultDeviceType(nvinfer1::DeviceType::kDLA);
            config->setDLACore(use_dla_core);
        }
    }

    static auto StreamDeleter = [](cudaStream_t *pStream) {
        if (pStream) {
            static_cast<void>(cudaStreamDestroy(*pStream));
            delete pStream;
        }
    };

    inline std::unique_ptr<cudaStream_t, decltype(StreamDeleter)> MakeCudaStream() {
        std::unique_ptr<cudaStream_t, decltype(StreamDeleter)> pStream(new cudaStream_t,
                                                                       StreamDeleter);
        if (cudaStreamCreateWithFlags(pStream.get(), cudaStreamNonBlocking) != cudaSuccess) {
            pStream.reset(nullptr);
        }
        return pStream;
    }

    static std::string GetTensorRTVersion() {
        return std::to_string(NV_TENSORRT_MAJOR) + "." + std::to_string(NV_TENSORRT_MINOR) + "."
               + std::to_string(NV_TENSORRT_PATCH);
    }

    static bool CheckEngineVersion(const std::string &engine_version_fpath,
                                   const std::string &cur_engine_version) {
        std::ifstream engine_version_file(engine_version_fpath);
        if (!engine_version_file.is_open()) {
            RS_LOGD("Open engine version file failed");
            return true;
        }
        std::string trt_version;
        std::string engine_version;
        std::getline(engine_version_file, trt_version);
        std::getline(engine_version_file, engine_version);
        engine_version_file.close();

        std::string cur_trt_version = GetTensorRTVersion();
        return trt_version == cur_trt_version && engine_version == cur_engine_version;
    }

    ErrorCode ReadEngineData(const std::string &filename, std::vector<char> &engine_data) {
        std::ifstream engine_file(filename, std::ios::binary);
        if (engine_file.fail()) {
            RS_LOGE("Open engine file failed");
            return ErrorCode::RS_INVALID_FILE;
        }

        engine_file.seekg(0, std::ifstream::end);
        auto fsize = engine_file.tellg();
        engine_file.seekg(0, std::ifstream::beg);

        engine_data.resize(fsize);
        engine_file.read(engine_data.data(), fsize);

        return ErrorCode::RS_SUCCESS;
    }

    ErrorCode TensorRTBuilder::Build(const std::string &onnx_data, const Config &config,
                                     std::vector<char> &engine_data) {
        initLibNvInferPlugins(&tensorrt::g_logger, "");

        std::string engine_fname = config.engine_fname;
        std::string cache_dir = config.cache_dir;
        if (engine_fname.empty()) {
            engine_fname = "trt_model.engine";
        }
        if (cache_dir.empty()) {
            cache_dir = ".cache";
        }
        std::string engine_version_fpath = utils::JoinPath(cache_dir, engine_fname + ".version");

        std::string engine_fpath;
        if (!cache_dir.empty()) {
            engine_fpath = utils::FindFileInDirectory(cache_dir, engine_fname, false);
            if (!engine_fpath.empty()) {
                if (CheckEngineVersion(engine_version_fpath, config.model_version)) {
                    return ReadEngineData(engine_fpath, engine_data);
                } else {
                    RS_LOGW("Match engine verion failed, regen engine file");
                }
            }
            ErrorCode err = utils::CreateDir(cache_dir, true);
            if (err != ErrorCode::RS_SUCCESS) {
                RS_LOGE("Create directory failed");
                return err;
            }
        }

        {
            engine_fpath = utils::JoinPath(cache_dir, engine_fname);
            RS_LOGD("engine_fpath: %s", engine_fpath.c_str());
#if NV_TENSORRT_MAJOR > 7
            std::string timing_cache_fpath;
            if (!config.timing_cache_fname.empty()) {
                timing_cache_fpath = utils::JoinPath(cache_dir, config.timing_cache_fname);
            }
#endif

            auto builder = tensorrt::TrtUniquePtr<nvinfer1::IBuilder>(
                nvinfer1::createInferBuilder(tensorrt::g_logger));
            if (!builder) {
                RS_LOGE("Create infer builder failed");
                return ErrorCode::RS_MODEL_COMPILE_ERROR;
            }

#if NV_TENSORRT_MAJOR >= 10
            nvinfer1::NetworkDefinitionCreationFlags flags = 0;
#else
            nvinfer1::NetworkDefinitionCreationFlags flags = 1;
#endif
            auto network = tensorrt::TrtUniquePtr<nvinfer1::INetworkDefinition>(
                builder->createNetworkV2(flags));
            if (!network) {
                RS_LOGE("Create networkv2 failed");
                return ErrorCode::RS_MODEL_COMPILE_ERROR;
            }

            auto builder_config =
                tensorrt::TrtUniquePtr<nvinfer1::IBuilderConfig>(builder->createBuilderConfig());
            if (!builder_config) {
                RS_LOGE("Create builder config failed");
                return ErrorCode::RS_MODEL_COMPILE_ERROR;
            }

            auto parser = tensorrt::TrtUniquePtr<nvonnxparser::IParser>(
                nvonnxparser::createParser(*network, tensorrt::g_logger));
            if (!parser) {
                RS_LOGE("Create onnx parser failed");
                return ErrorCode::RS_MODEL_COMPILE_ERROR;
            }

#if NV_TENSORRT_MAJOR > 7
            auto timing_cache = std::unique_ptr<nvinfer1::ITimingCache>();
#endif

            {
                auto parsed = parser->parse(onnx_data.data(), onnx_data.size());
                // auto parsed = parser->parseFromFile(
                //     "/nvme2/medsam/projects/rayshape_deploy/model/breast_thyroid/onnx/checkpoint-best.onnx",
                //     static_cast<int>(nvinfer1::ILogger::Severity::kVERBOSE));
                if (!parsed) {
                    RS_LOGE("Parse onnx model failed");
                    return ErrorCode::RS_MODEL_COMPILE_ERROR;
                }

                switch (config.precision) {
                case TensorRTBuilder::Precision::FP32: {
                    break;
                }
                case TensorRTBuilder::Precision::FP16: {
                    if (builder->platformHasFastFp16()) {
                        RS_LOGD("Platform has fast fp16\n");
                        builder_config->setFlag(nvinfer1::BuilderFlag::kFP16);
                    }
                    break;
                }
                case TensorRTBuilder::Precision::INT8: {
                    if (builder->platformHasFastInt8()) {
                        RS_LOGD("Platform has fast int8\n");
                        builder_config->setFlag(nvinfer1::BuilderFlag::kINT8);
                        if (config.int8_calibrator) {
                            builder_config->setInt8Calibrator(config.int8_calibrator.get());
                        } else {
                            RS_LOGE("Int8 calibrator is nullptr\n");
                            return ErrorCode::RS_INVALID_PARAM_VALUE;
                        }
                    }
                    break;
                }
                }

#if NV_TENSORRT_MAJOR > 7
                builder_config->setProfilingVerbosity(config.profiling_verbosity);
                builder_config->setBuilderOptimizationLevel(config.opt_level);
#else
                builder_config->setMaxWorkspaceSize(3ULL << 30); // 3GB
#endif
                if (config.disable_tf32) {
                    builder_config->clearFlag(nvinfer1::BuilderFlag::kTF32);
                }

#if NV_TENSORRT_MAJOR > 7
                if (!timing_cache_fpath.empty()) {
                    timing_cache = tensorrt::BuildTimingCacheFromFile(
                        tensorrt::g_logger, *builder_config, timing_cache_fpath);
                }
#endif

                EnableDLA(builder.get(), builder_config.get(), config.use_dla_core);
            }

            // CUDA stream used for profiling by the builder.
            auto profileStream = MakeCudaStream();
            if (!profileStream) {
                RS_LOGE("Make cuda stream failed");
                return ErrorCode::RS_MODEL_COMPILE_ERROR;
            }
            builder_config->setProfileStream(*profileStream);

#if NV_TENSORRT_MAJOR > 7
            std::unique_ptr<nvinfer1::IHostMemory> plan{
                builder->buildSerializedNetwork(*network, *builder_config)};
            if (!plan) {
                RS_LOGE("Build serialized network failed");
                return ErrorCode::RS_MODEL_COMPILE_ERROR;
            }
            if (!timing_cache_fpath.empty()) {
                ErrorCode err = tensorrt::UpdateTimingCacheFile(
                    tensorrt::g_logger, timing_cache_fpath, timing_cache.get(), *builder);
                if (err != ErrorCode::RS_SUCCESS) {
                    RS_LOGE("Update timing cache file failed");
                    return err;
                }
            }
#else
            auto engine = tensorrt::TrtUniquePtr<nvinfer1::ICudaEngine>(
                builder->buildEngineWithConfig(*network, *builder_config));
            if (!engine) {
                RS_LOGE("Build engine failed");
                return ErrorCode::RS_MODEL_COMPILE_ERROR;
            }
            auto plan = tensorrt::TrtUniquePtr<nvinfer1::IHostMemory>(engine->serialize());
#endif

            engine_data.resize(plan->size());
            memcpy(engine_data.data(), plan->data(), plan->size());

            std::ofstream engine_file(engine_fpath, std::ios::binary);
            if (!engine_file.is_open()) {
                RS_LOGE("Failed to open file for writing engine.\n");
                return ErrorCode::RS_INVALID_FILE;
            }
            engine_file.write(reinterpret_cast<const char *>(plan->data()), plan->size());
            engine_file.close();

            std::ofstream engine_version_file(engine_version_fpath);
            if (!engine_version_file.is_open()) {
                RS_LOGE("Failed to open file for writing engine version.\n");
                return ErrorCode::RS_INVALID_FILE;
            }
            engine_version_file << GetTensorRTVersion() << "\n" << config.model_version;
            engine_version_file.close();
        }

        return ErrorCode::RS_SUCCESS;
    }
} // namespace rayshape