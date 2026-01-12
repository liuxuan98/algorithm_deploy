#include "inference/tensorrt/tensorrt_logger.h"

#include <string>

#include "base/logger.h"

namespace rayshape
{
    namespace tensorrt
    {
        void TensorRTLogger::log(nvinfer1::ILogger::Severity severity, const char *msg) noexcept {
            // suppress info-level messages
#ifndef RSDEPLOY_DEBUG
            if (severity == Severity::kINFO || severity == Severity::kVERBOSE)
                return;
#endif
            const char *skips[] = {
                "INVALID_ARGUMENT: Cannot find binding of given name",
                "Unused Input:",
                "Detected invalid timing cache",
                "unused or used only at compile-time",
            };

            std::string msg_str = std::string(msg);
            for (auto skip : skips) {
                if (msg_str.find(skip) != std::string::npos) {
                    return;
                }
            }
            switch (severity) {
            case Severity::kINTERNAL_ERROR:
                RS_LOGE("INTERNAL_ERROR: %s\n", msg);
                break;
            case Severity::kERROR:
                RS_LOGE("ERROR: %s\n", msg);
                break;
            case Severity::kWARNING:
                RS_LOGW("%s\n", msg);
                break;
            case Severity::kINFO:
                RS_LOGI("%s\n", msg);
                break;
            case Severity::kVERBOSE:
                RS_LOGI("VERBOSE: %s\n", msg);
                break;
            default:
                break;
            }
        }

        TensorRTLogger g_logger;
    } // namespace tensorrt
} // namespace rayshape