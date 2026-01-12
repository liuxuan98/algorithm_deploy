#include "inference/onnxruntime/onnxruntime_config_converter.h"
#include "base/logger.h"

namespace rayshape
{
    namespace onnxruntime
    {

        DataType ONNXRuntimeConfigConverter::ConvertToDataType(
            ONNXTensorElementDataType precision) {
            if (ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT == precision) {
                return DataType::FLOAT;
            } else if (ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT16
                       == precision) {
                return DataType::HALF;
            } else if (ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_INT32
                       == precision) {
                return DataType::INT32;
            } else if (ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_INT64
                       == precision) {
                return DataType::INT64;
            } else if (ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_INT8 == precision) {
                return DataType::INT8;
            } else if (ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT32
                       == precision) {
                return DataType::UINT32;
            } else {
                RS_LOGE("ONNXRuntimeConfigConverter::ConvertToDataType precision is not exist.\n");
                return DataType::AUTO;
            }
            return DataType::AUTO;
        }

        ErrorCode ONNXRuntimeConfigConverter::ConvertFromDataType(
            ONNXTensorElementDataType &precision, const DataType dataType) {
            if (dataType == DataType::FLOAT) {
                precision = ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT;
            } else if (dataType == DataType::HALF) {
                precision = ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT16;
            } else if (dataType == DataType::INT32) {
                precision = ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_INT32;
            } else if (dataType == DataType::INT64) {
                precision = ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_INT64;
            } else if (dataType == DataType::INT8) {
                precision = ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_INT8;
            } else if (dataType == DataType::UINT32) {
                precision = ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT32;
            } else {
                RS_LOGE(
                    "ONNXRuntimeConfigConverter ConvertFromDataType DataType:%d is not exist.\n",
                    dataType);
                return RS_INVALID_PARAM_FORMAT;
            }

            return ErrorCode::RS_SUCCESS;
        }

        ErrorCode ONNXRuntimeConfigConverter::ConvertFromDims(std::vector<int64_t> &dst,
                                                              const Dims &src) {
            dst.resize(src.size);
            for (int i = 0; i < src.size; i++) {
                dst[i] = static_cast<int64_t>(src.value[i]);
            }

            return ErrorCode::RS_SUCCESS;
        }

        ErrorCode ONNXRuntimeConfigConverter::ConvertToDims(Dims &dst,
                                                            const std::vector<int64_t> &src) {
            if (src.size() > (sizeof(dst.value) / sizeof(int))) {
                RS_LOGE("ONNXRuntimeConfigConverter::ConvertToDims src.size():%zu is too large.\n",
                        src.size());
                return ErrorCode::RS_INVALID_PARAM_VALUE;
            }

            dst.size = src.size();
            for (int i = 0; i < src.size(); i++) {
                dst.value[i] = static_cast<int>(src[i]);
            }
            return ErrorCode::RS_SUCCESS;
        }

        ErrorCode ONNXRuntimeConfigConverter::ConvertToDevice(DeviceType &dst,
                                                              const std::string &src) {
            if (src == "CPU") {
                dst = DeviceType::X86;
            } else if (src == "CUDA") {
                dst = DeviceType::CUDA;
            } else {
                dst = DeviceType::X86;
            }

            return ErrorCode::RS_SUCCESS;
        }
        ErrorCode ONNXRuntimeConfigConverter::ConvertFromDevice(std::string &dst,
                                                                const DeviceType &src) {
            if (DeviceType::X86 == src) {
                dst = "CPU";
            } else if (DeviceType::CUDA == src) {
                dst = "CUDA";
            } else {
                dst = "CPU";
            }
            return ErrorCode::RS_SUCCESS;
        }

        DataFormat ONNXRuntimeConfigConverter::ConvertToDataFormat(const std::string &src) {
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
    } // namespace onnxruntime
} // namespace rayshape
