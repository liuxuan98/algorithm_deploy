#ifndef _TENSORRT_LOGGER_H_
#define _TENSORRT_LOGGER_H_

#include "inference/tensorrt/tensorrt_include.h"

namespace rayshape
{
    namespace tensorrt
    {
        class TensorRTLogger: public nvinfer1::ILogger {
        public:
            void log(nvinfer1::ILogger::Severity severity, const char *msg) noexcept override;
        };

        extern TensorRTLogger g_logger;
    } // namespace tensorrt
} // namespace rayshape

#endif // _TENSORRT_LOGGER_H_