#ifndef _ONNXRUNTIME_CONFIG_CONVERTER_H_
#define _ONNXRUNTIME_CONFIG_CONVERTER_H_

#include "memory_manager/blob.h"
#include "onnxruntime_include.h"

namespace rayshape
{
    namespace onnxruntime
    {

        class ONNXRuntimeConfigConverter {
        public: // Fp16 ,uint8 ,fp32
            static DataType ConvertToDataType(ONNXTensorElementDataType precision);

            static ErrorCode ConvertFromDataType(ONNXTensorElementDataType &precision,
                                                 const DataType dataType);

            static ErrorCode ConvertFromDims(std::vector<int64_t> &dst, const Dims &src);

            static ErrorCode ConvertToDims(Dims &dst, const std::vector<int64_t> &src);

            static ErrorCode ConvertToDevice(DeviceType &dst, const std::string &src);
            static ErrorCode ConvertFromDevice(std::string &dst, const DeviceType &src);
            //
            static DataFormat ConvertToDataFormat(const std::string &src);
        };

    } // namespace onnxruntime
} // namespace rayshape

#endif