#ifndef OPENVINO_CONFIG_CONVERTER_H
#define OPENVINO_CONFIG_CONVERTER_H

#include "memory_manager/blob.h"
#include "openvino_include.h"

namespace rayshape
{
    namespace inference
    {
        namespace openvino
        {
            class OpenVINOConfigConverter {
            public: // Fp16 ,uint8 ,fp32
                static DataType ConvertToDataType(const ov::element::Type &precision);

                static ErrorCode ConvertFromDataType(ov::element::Type &precision,
                                                     const DataType data_type);

                static ErrorCode ConvertFromDims(ov::Shape &dst, const Dims &src);

                static ErrorCode ConvertToDims(Dims &dst, const ov::Shape &src);

                static ErrorCode ConvertToDevice(DeviceType &dst, const std::string &src);
                static ErrorCode ConvertFromDevice(std::string &dst, const DeviceType &src);
                //
                static DataFormat ConvertToDataFormat(const std::string &src);
                static ErrorCode ConvertFromDataFormat(ov::Layout &dst, const DataFormat src);
            };
        } // namespace openvino
    }     // namespace inference
} // namespace rayshape

#endif // OPENVINO_CONFIG_CONVERTER_H
