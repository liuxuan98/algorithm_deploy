#include "inference/mnn/mnn_config_converter.h"
#include "base/logger.h"

namespace rayshape
{
    namespace mnn
    {
        DataType MNNConfigConverter::ConvertToDataType(const halide_type_t &halide_type) {
            // TODO: Implement MNN halide type to RayShape DataType conversion
            if (halide_type.code == halide_type_float && halide_type.bits == 32) {
                return DataType::FLOAT;
            } else if (halide_type.code == halide_type_float && halide_type.bits == 16) {
                return DataType::HALF;
            } else if (halide_type.code == halide_type_int && halide_type.bits == 32) {
                return DataType::INT32;
            } else if (halide_type.code == halide_type_int && halide_type.bits == 64) {
                return DataType::INT64;
            } else if (halide_type.code == halide_type_uint && halide_type.bits == 32) {
                return DataType::UINT32;
            } else if (halide_type.code == halide_type_uint && halide_type.bits == 8) {
                return DataType::UINT8;
            } else if (halide_type.code == halide_type_int && halide_type.bits == 8) {
                return DataType::INT8;
            } else {
                RS_LOGE("MNNConfigConverter::ConvertToDataType halide_type is not supported.\n");
                return DataType::AUTO;
            }
        }

        ErrorCode MNNConfigConverter::ConvertFromDataType(halide_type_t &halide_type,
                                                          const DataType data_type) {
            switch (data_type) {
            case DataType::FLOAT:
                halide_type.code = halide_type_float;
                halide_type.bits = 32;
                halide_type.lanes = 1;
                break;
            case DataType::HALF:
                halide_type.code = halide_type_float;
                halide_type.bits = 16;
                halide_type.lanes = 1;
                break;
            case DataType::INT32:
                halide_type.code = halide_type_int;
                halide_type.bits = 32;
                halide_type.lanes = 1;
                break;
            case DataType::INT64:
                halide_type.code = halide_type_int;
                halide_type.bits = 64;
                halide_type.lanes = 1;
                break;
            case DataType::UINT32:
                halide_type.code = halide_type_uint;
                halide_type.bits = 32;
                halide_type.lanes = 1;
                break;
            case DataType::UINT8:
                halide_type.code = halide_type_uint;
                halide_type.bits = 8;
                halide_type.lanes = 1;
                break;
            case DataType::INT8:
                halide_type.code = halide_type_int;
                halide_type.bits = 8;
                halide_type.lanes = 1;
                break;
            default:
                RS_LOGE("MNNConfigConverter::ConvertFromDataType data_type is not supported.\n");
                return RS_INVALID_PARAM_FORMAT;
            }
            return RS_SUCCESS;
        }

        ErrorCode MNNConfigConverter::ConvertToDims(Dims &dst, const std::vector<int> &src) {
            // TODO: Implement MNN shape to RayShape Dims conversion
            dst.size = static_cast<int>(src.size());
            if (dst.size > MAX_DIMS_SIZE) {
                RS_LOGE(
                    "MNNConfigConverter::ConvertToDims dims size:%d exceeds MAX_DIMS_SIZE:%d.\n",
                    dst.size, MAX_DIMS_SIZE);
                return RS_INVALID_PARAM;
            }

            for (int i = 0; i < dst.size; i++) {
                dst.value[i] = src[i];
            }
            return RS_SUCCESS;
        }

        ErrorCode MNNConfigConverter::ConvertFromDims(std::vector<int> &dst, const Dims &src) {
            // TODO: Implement RayShape Dims to MNN shape conversion
            dst.clear();
            for (int i = 0; i < src.size; i++) {
                dst.emplace_back(src.value[i]);
            }
            return RS_SUCCESS;
        }

        ErrorCode MNNConfigConverter::ConvertToDevice(DeviceType &dst, const std::string &src) {
            if (src == "CPU" || src == "cpu") {
                dst = DeviceType::X86;
            } else if (src == "GPU" || src == "gpu" || src == "OPENCL" || src == "opencl") {
                dst = DeviceType::OPENCL;
            } else if (src == "ARM" || src == "arm" || src == "VULKAN" || src == "vulkan") {
                dst = DeviceType::ARM;
            } else {
                dst = DeviceType::X86;
            }
            return RS_SUCCESS;
        }

        ErrorCode MNNConfigConverter::ConvertFromDevice(const DeviceType &device_type,
                                                        MNN::BackendConfig &backend_config,
                                                        MNNForwardType &forward_type) {
            switch (device_type) {
            case DeviceType::ARM:
                forward_type = MNN_FORWARD_CPU;
                backend_config.memory = MNN::BackendConfig::Memory_Normal;
                backend_config.power = MNN::BackendConfig::Power_Normal;
                backend_config.precision = MNN::BackendConfig::Precision_Low;
                break;

            case DeviceType::OPENCL:
                forward_type = MNN_FORWARD_OPENCL;
                backend_config.memory = MNN::BackendConfig::Memory_Normal;
                backend_config.power = MNN::BackendConfig::Power_Normal;
                backend_config.precision = MNN::BackendConfig::Precision_Low;
                break;

            case DeviceType::CUDA:
                forward_type = MNN_FORWARD_CUDA;
                backend_config.memory = MNN::BackendConfig::Memory_Normal;
                backend_config.power = MNN::BackendConfig::Power_Normal;
                backend_config.precision = MNN::BackendConfig::Precision_Low;
                break;

            default:
                forward_type = MNN_FORWARD_CPU;
                backend_config.memory = MNN::BackendConfig::Memory_Normal;
                backend_config.power = MNN::BackendConfig::Power_Normal;
                backend_config.precision = MNN::BackendConfig::Precision_Normal;
                break;
            }

            return RS_SUCCESS;
        }

        DataFormat MNNConfigConverter::ConvertToDataFormat(const std::string &src) {
            if (src == "NCHW" || src == "nchw") {
                return DataFormat::NCHW;
            } else if (src == "NHWC" || src == "nhwc") {
                return DataFormat::NHWC;
            } else if (src == "NC" || src == "nc") {
                return DataFormat::NC;
            } else if (src == "NCDHW" || src == "ncdhw") {
                return DataFormat::NCDHW;
            } else {
                return DataFormat::AUTO;
            }
        }

        ErrorCode MNNConfigConverter::ConvertFromDataFormat(MNN::Tensor::DimensionType &dst,
                                                            const DataFormat src) {
            switch (src) {
            case DataFormat::NCHW:
                dst = MNN::Tensor::TENSORFLOW;
                break;
            case DataFormat::NHWC:
                dst = MNN::Tensor::CAFFE;
                break;
            case DataFormat::NC:
                dst = MNN::Tensor::CAFFE_C4;
                break;
            case DataFormat::NCDHW:
                dst = MNN::Tensor::CAFFE_C4;
                break;
            default:
                dst = MNN::Tensor::TENSORFLOW;
                break;
            }
            return RS_SUCCESS;
        }

        ErrorCode MNNConfigConverter::GetDefaultSessionConfig(MNN::ScheduleConfig &config) {
            config.type = MNN_FORWARD_CPU;
            config.numThread = 4;
            config.backupType = MNN_FORWARD_CPU;
            config.backendConfig = nullptr;

            return RS_SUCCESS;
        }

        ErrorCode MNNConfigConverter::ConvertFromSessionConfig(
            MNN::ScheduleConfig &dst, const std::map<std::string, std::string> &src) {
            // Initialize with default values
            GetDefaultSessionConfig(dst);

            // Override with provided configuration
            for (const auto &item : src) {
                if (item.first == "numThread") {
                    try {
                        dst.numThread = std::stoi(item.second);
                    } catch (const std::exception &e) {
                        RS_LOGE("Failed to parse numThread: %s\n", e.what());
                        return RS_INVALID_PARAM_FORMAT;
                    }
                }
                // Add more configuration options as needed
            }

            return RS_SUCCESS;
        }

        ErrorCode MNNConfigConverter::ConvertTensorInfo(MNN::Tensor *mnn_tensor,
                                                        Blob *rayshape_blob) {
            if (mnn_tensor == nullptr || rayshape_blob == nullptr) {
                return RS_INVALID_PARAM;
            }

            // Convert dimensions
            auto shape = mnn_tensor->shape();
            rayshape_blob->dims.size = static_cast<int>(shape.size());
            for (size_t i = 0; i < shape.size() && i < MAX_DIMS_SIZE; i++) {
                rayshape_blob->dims.value[i] = shape[i];
            }

            // Convert data type
            auto halide_type = mnn_tensor->getType();
            rayshape_blob->data_type = ConvertToDataType(halide_type);

            // Convert data format
            auto dimension_type = mnn_tensor->getDimensionType();
            // Convert MNN dimension type to DataFormat directly
            if (dimension_type == MNN::Tensor::TENSORFLOW) {
                rayshape_blob->data_format = DataFormat::NCHW;
            } else if (dimension_type == MNN::Tensor::CAFFE) {
                rayshape_blob->data_format = DataFormat::NHWC;
            } else if (dimension_type == MNN::Tensor::CAFFE_C4) {
                rayshape_blob->data_format = DataFormat::NC;
            } else {
                rayshape_blob->data_format = DataFormat::AUTO;
            }

            return RS_SUCCESS;
        }
    } // namespace mnn
} // namespace rayshape
