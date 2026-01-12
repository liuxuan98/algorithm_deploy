#ifndef TENSORRT_BUILDER_H
#define TENSORRT_BUILDER_H

#include <string>
#include <memory>

#include "base/macros.h"
#include "base/error.h"
#include "inference/tensorrt/tensorrt_include.h"

namespace rayshape
{
    class RS_PUBLIC TensorRTBuilder {
    public:
        enum class Precision { FP32, FP16, INT8 };

        struct Config {
            std::string model_version;
            Precision precision = Precision::FP32;
#if NV_TENSORRT_MAJOR > 7
            nvinfer1::ProfilingVerbosity profiling_verbosity =
                nvinfer1::ProfilingVerbosity::kLAYER_NAMES_ONLY;
            int opt_level = 3;
#endif
            bool disable_tf32 = false;
            int use_dla_core = -1;
            std::string engine_fname;
#if NV_TENSORRT_MAJOR > 7
            std::string timing_cache_fname;
#endif
            std::string cache_dir;

            std::unique_ptr<nvinfer1::IInt8Calibrator> int8_calibrator = nullptr;
        };

        static ErrorCode Build(const std::string &onnx_data, const Config &config,
                               std::vector<char> &engine_data);
    };
} // namespace rayshape

#endif