#ifndef _TENSORRT_CONFIG_CONVERTER_H_
#define _TENSORRT_CONFIG_CONVERTER_H_

#include "memory_manager/blob.h"
#include "tensorrt_include.h"

namespace rayshape
{
    namespace tensorrt
    {

        class TensorRTConfigConverter {
        public: // Fp16 ,uint8 ,fp32
            static DataType ConvertToDataType(const nvinfer1::DataType &precision);

            static ErrorCode ConvertFromDataType(nvinfer1::DataType &precision,
                                                 const DataType dataType);

            static ErrorCode ConvertFromDims(nvinfer1::Dims &dst, const Dims &src);

            static ErrorCode ConvertToDims(Dims &dst, const nvinfer1::Dims &src);

            static ErrorCode ConvertToDevice(DeviceType &dst, const std::string &src);
            static ErrorCode ConvertFromDevice(std::string &dst, const DeviceType &src);
            //
            static DataFormat ConvertToDataFormat(const std::string &src);
            static ErrorCode ConvertFromDataFormat(nvinfer1::TensorFormat &dst,
                                                   const DataFormat src);
        };

    } // namespace tensorrt
} // namespace rayshape

#endif