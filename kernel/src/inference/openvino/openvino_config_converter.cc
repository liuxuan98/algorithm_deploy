#include "inference/openvino/openvino_config_converter.h"
#include "base/logger.h"

namespace rayshape
{
    namespace inference
    {
        namespace openvino
        {
            DataType OpenVINOConfigConverter::ConvertToDataType(
                const ov::element::Type &precision) {
                if (ov::element::f32 == precision) {
                    return DataType::FLOAT;
                } else if (ov::element::f16 == precision) {
                    return DataType::HALF;
                } else if (ov::element::i32 == precision) {
                    return DataType::INT32;
                } else if (ov::element::i64 == precision) {
                    return DataType::INT64;
                } else if (ov::element::u32 == precision) {
                    return DataType::UINT32;
                } else if (ov::element::u8 == precision) {
                    return DataType::INT8;
                } else {
                    RS_LOGE("OpenVINOConfigConverter::ConvertToDataType precision is not exist.\n");
                    return DataType::AUTO;
                }
                return DataType::AUTO;
            }

            ErrorCode OpenVINOConfigConverter::ConvertFromDataType(ov::element::Type &precision,
                                                                   const DataType data_type) {
                if (data_type == DataType::FLOAT) {
                    precision = ov::element::f32;
                } else if (data_type == DataType::HALF) {
                    precision = ov::element::f16;
                } else if (data_type == DataType::INT32) {
                    precision = ov::element::i32;
                } else if (data_type == DataType::INT64) {
                    precision = ov::element::i64;
                } else if (data_type == DataType::UINT32) {
                    precision = ov::element::u32;
                } else if (data_type == DataType::INT8) {
                    precision = ov::element::u8;
                } else {
                    RS_LOGE(
                        "OpenVINOConfigConverter ConvertFromDataType DataType:%d is not exist.\n",
                        static_cast<int>(data_type));
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
                                                               const std::string &ov_device_name) {
                if (ov_device_name == "CPU") {
                    dst = DeviceType::X86;
                } else if (ov_device_name == "GPU") {
                    dst = DeviceType::OPENCL;
                } else {
                    dst = DeviceType::X86;
                }

                return RS_SUCCESS;
            }

            ErrorCode OpenVINOConfigConverter::ConvertFromDevice(std::string &dst,
                                                                 const DeviceType &src) {
                if (DeviceType::X86 == src) {
                    dst = std::string("CPU");
                } else if (DeviceType::OPENCL == src) {
                    dst = std::string("GPU");
                } else if (DeviceType::INTERL_NPU == src) {
                    dst = std::string("NPU");
                } else {
                    dst = std::string("CPU");
                }

                return RS_SUCCESS;
            }

            DataFormat OpenVINOConfigConverter::ConvertToDataFormat(const std::string &layout) {
                if (layout == "NCHW") {
                    return DataFormat::NCHW;
                } else if (layout == "NHWC") {
                    return DataFormat::NHWC;
                } else if (layout == "NC") {
                    return DataFormat::NC;
                } else if (layout == "NCDHW") {
                    return DataFormat::NCDHW;
                } else {
                    return DataFormat::AUTO;
                }
            }

            ErrorCode OpenVINOConfigConverter::ConvertFromDataFormat(ov::Layout &dst,
                                                                     DataFormat src) {
                switch (src) {
                case DataFormat::NCHW:
                    dst = ov::Layout("NCHW");
                    break;
                case DataFormat::NHWC:
                    dst = ov::Layout("NHWC");
                    break;
                case DataFormat::NC:
                    dst = ov::Layout("NC");
                    break;
                case DataFormat::NCDHW:
                    dst = ov::Layout("NCDHW");
                    break;
                default:
                    dst = ov::Layout("NCHW");
                    break;
                }
                return RS_SUCCESS;
            }

        } // namespace openvino
    }     // namespace inference
} // namespace rayshape
