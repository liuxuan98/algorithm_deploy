#include "inference/tensorrt/tensorrt_config_converter.h"
#include "base/logger.h"

namespace rayshape
{
    namespace tensorrt
    {

        DataType TensorRTConfigConverter::ConvertToDataType(const nvinfer1::DataType &precision) {
            if (nvinfer1::DataType::kFLOAT == precision) {
                return DataType::FLOAT;
            } else if (nvinfer1::DataType::kHALF == precision) {
                return DataType::HALF;
            } else if (nvinfer1::DataType::kINT32 == precision) {
                return DataType::INT32;
            }
#if NV_TENSORRT_MAJOR >= 10
            else if (nvinfer1::DataType::kINT64 == precision) {
                return DataType::INT64;
            }
#endif
            else if (nvinfer1::DataType::kINT8 == precision) {
                return DataType::INT8;
            } else {
                RS_LOGE("TensorRTConfigConverter::ConvertToDataType precision is not exist.\n");
                return DataType::AUTO;
            }
            return DataType::AUTO;
        }

        ErrorCode TensorRTConfigConverter::ConvertFromDataType(nvinfer1::DataType &precision,
                                                               const DataType dataType) {
            if (dataType == DataType::FLOAT) {
                precision = nvinfer1::DataType::kFLOAT;
            } else if (dataType == DataType::HALF) {
                precision = nvinfer1::DataType::kHALF;
            } else if (dataType == DataType::INT32) {
                precision = nvinfer1::DataType::kINT32;
            }
#if NV_TENSORRT_MAJOR >= 10
            else if (dataType == DataType::INT64) {
                precision = nvinfer1::DataType::kINT64;
            }
#endif
            else if (dataType == DataType::INT8) {
                precision = nvinfer1::DataType::kINT8;
            } else {
                RS_LOGE("TensorRTConfigConverter ConvertFromDataType DataType:%d is not exist.\n",
                        dataType);
                return RS_INVALID_PARAM_FORMAT;
            }

            return RS_SUCCESS;
        }

        ErrorCode TensorRTConfigConverter::ConvertFromDims(nvinfer1::Dims &dst, const Dims &src) {
            if (src.size > nvinfer1::Dims::MAX_DIMS) {
                RS_LOGE("TensorRTConfigConverter::ConvertFromDims src.size:%d is too large.\n",
                        src.size);
                return ErrorCode::RS_INVALID_PARAM_VALUE;
            }

            dst.nbDims = src.size;
            for (int i = 0; i < src.size; i++) {
                dst.d[i] = static_cast<int64_t>(src.value[i]);
            }

            return RS_SUCCESS;
        }

        ErrorCode TensorRTConfigConverter::ConvertToDims(Dims &dst, const nvinfer1::Dims &src) {
            if (src.nbDims > (sizeof(dst.value) / sizeof(int))) {
                RS_LOGE("TensorRTConfigConverter::ConvertToDims src.nbDims:%d is too large.\n",
                        src.nbDims);
                return ErrorCode::RS_INVALID_PARAM_VALUE;
            }

            dst.size = src.nbDims;
            for (int i = 0; i < src.nbDims; i++) {
                dst.value[i] = static_cast<int>(src.d[i]);
            }
            return RS_SUCCESS;
        }

        ErrorCode TensorRTConfigConverter::ConvertToDevice(DeviceType &dst,
                                                           const std::string &src) {
            if (src == "CPU") {
                dst = DeviceType::X86;
            } else if (src == "CUDA") {
                dst = DeviceType::CUDA;
            } else {
                dst = DeviceType::X86;
            }

            return RS_SUCCESS;
        }
        ErrorCode TensorRTConfigConverter::ConvertFromDevice(std::string &dst,
                                                             const DeviceType &src) {
            if (DeviceType::X86 == src) {
                dst = "CPU";
            } else if (DeviceType::CUDA == src) {
                dst = "CUDA";
            } else {
                dst = "CPU";
            }
            return RS_SUCCESS;
        }

        DataFormat TensorRTConfigConverter::ConvertToDataFormat(const std::string &src) {
            if ("NCHW" == src) {
                return DataFormat::NCHW;
            } else if ("NHWC" == src) {
                return DataFormat::NHWC;
            } else if ("NC" == src) {
                return DataFormat::NC;
            } else if ("NCDHW" == src) {
                return DataFormat::NCDHW;
            } else {
                return DataFormat::AUTO;
            } // 暂时先列举这些数据格式
        }

        ErrorCode TensorRTConfigConverter::ConvertFromDataFormat(nvinfer1::TensorFormat &dst,
                                                                 const DataFormat src) {
            switch (src) {
            case DataFormat::NC:
            case DataFormat::NCHW:
            case DataFormat::NCDHW:
                dst = nvinfer1::TensorFormat::kLINEAR;
                break;
#if NV_TENSORRT_MAJOR > 7
            case DataFormat::NHWC:
                dst = nvinfer1::TensorFormat::kHWC;
                break;
            case DataFormat::NHWC4:
                dst = nvinfer1::TensorFormat::kDLA_HWC4;
                break;
#endif
            default:
                return RS_INVALID_PARAM_VALUE;
            }

            return RS_SUCCESS;
        }

    } // namespace tensorrt
} // namespace rayshape
