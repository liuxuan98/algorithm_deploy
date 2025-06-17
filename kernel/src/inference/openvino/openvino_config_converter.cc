#include "inference/openvino/openvino_config_converter.h"
#include "base/logger.h"

namespace rayshape
{
    namespace openvino
    {
        DataType OpenVINOConfigConverter::ConvertToDataType(const ov::element::Type &precision) {
            if (ov::element::f32 == precision) {
                return DataType::DATA_TYPE_FLOAT;
            } else if (ov::element::f16 == precision) {
                return DataType::DATA_TYPE_HALF;
            } else if (ov::element::i32 == precision) {
                return DATA_TYPE_INT32;
            } else if (ov::element::i64 == precision) {
                return DATA_TYPE_INT64;
            } else if (ov::element::u32 == precision) {
                return DATA_TYPE_UINT32;
            } else if (ov::element::u8 == precision) {
                return DATA_TYPE_INT8;
            } else {
                RS_LOGE("OpenVINOConfigConverter::ConvertToDataType precision is not exist.\n", );
                return DATA_TYPE_AUTO;
            }
            return DATA_TYPE_AUTO;
        }

        ErrorCode OpenVINOConfigConverter::ConvertFromDataType(ov::element::Type &precision,
                                                               const DataType dataType) {
            if (dataType == DATA_TYPE_FLOAT) {
                precision = ov::element::f32;
            } else if (dataType == DATA_TYPE_HALF) {
                precision = ov::element::f16;
            } else if (dataType == DATA_TYPE_INT32) {
                precision = ov::element::i32;
            } else if (dataType == DATA_TYPE_INT64) {
                precision = ov::element::i64;
            } else if (dataType == DATA_TYPE_UINT32) {
                precision = ov::element::u32;
            } else if (dataType == DATA_TYPE_INT8) {
                precision = ov::element::u8;
            } else {
                // log
                RS_LOGE("OpenVINOConfigConverter ConvertFromDataType DataType:%d is not exist.\n",
                        dataType);
                return RS_INVALID_PARAM_FORMAT;
            }

            return RS_SUCCESS;
        }

        ErrorCode OpenVINOConfigConverter::ConvertFromDims(ov::Shape &dst, const Dims &src) {
            for (int i = 0; i < src.size; i++) {
                dst.emplace_back(static_cast<size_t>(src.value[i]));
            }

            return RS_SUCCESS;
        }

        ErrorCode OpenVINOConfigConverter::ConvertToDims(Dims &dst, const ov::Shape &src) {
            dst.size = src.size();
            for (int i = 0; i < src.size(); i++) {
                dst.value[i] = src[i];
            }
            return RS_SUCCESS;
        }

        ErrorCode OpenVINOConfigConverter::ConvertToDevice(DeviceType &dst,
                                                           const std::string &src) {
            if (src == "CPU") {
                dst = DEVICE_TYPE_X86;
            } else if (src == "GPU") {
                dst = DEVICE_TYPE_OPENCL;
            } else {
                dst = DEVICE_TYPE_X86;
            }

            return RS_SUCCESS;
        }
        ErrorCode OpenVINOConfigConverter::ConvertFromDevice(std::string &dst,
                                                             const DeviceType &src) {
            if (DEVICE_TYPE_X86 == src) {
                dst = "CPU";
            } else if (DEVICE_TYPE_OPENCL == src) {
                dst = "GPU";
            } else if (DEVICE_TYPE_INTERL_NPU == src) {
                dst = "AUTO:NPU,CPU";
            } else {
                dst = "CPU";
            }
            return RS_SUCCESS;
        }

        DataFormat OpenVINOConfigConverter::ConvertToDataFormat(const std::string &src) {
            if ("NCHW" == src) {
                return DATA_FORMAT_NCHW;
            } else if ("NHWC" == src) {
                return DATA_FORMAT_NHWC;
            } else if ("NC" == src) {
                return DATA_FORMAT_NC;
            } else if ("NCDHW" == src) {
                return DATA_FORMAT_NCDHW;
            } else {
                return DATA_FORMAT_AUTO;
            } // 暂时先列举这些数据格式
        }

        ErrorCode OpenVINOConfigConverter::ConvertFromDataFormat(ov::Layout &dst,
                                                                 const DataFormat src) {
            switch (src) {
            case DATA_FORMAT_NCHW:
                dst = ov::Layout("NCHW");
                break;
            case DATA_FORMAT_NHWC:
                dst = ov::Layout("NHWC");
                break;
            case DATA_FORMAT_NC:
                dst = ov::Layout("NC");
                break;
            case DATA_FORMAT_NCDHW:
                dst = ov::Layout("NCDHW");
                break;
            }

            return RS_SUCCESS;
        }

    } // namespace openvino
} // namespace rayshape
