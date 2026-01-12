#ifndef _MNN_CONFIG_CONVERTER_H_
#define _MNN_CONFIG_CONVERTER_H_

#include "memory_manager/blob.h"
#include "mnn_include.h"

namespace rayshape
{
    namespace mnn
    {
        class MNNConfigConverter {
        public:
            // Convert MNN data type to RayShape data type
            static DataType ConvertToDataType(const halide_type_t &halide_type);

            // Convert RayShape data type to MNN data type
            static ErrorCode ConvertFromDataType(halide_type_t &halide_type,
                                                 const DataType dataType);

            // Convert MNN shape to RayShape dims
            static ErrorCode ConvertToDims(Dims &dst, const std::vector<int> &src);

            // Convert RayShape dims to MNN shape
            static ErrorCode ConvertFromDims(std::vector<int> &dst, const Dims &src);

            // Convert device type from string to RayShape device type
            static ErrorCode ConvertToDevice(DeviceType &dst, const std::string &src);

            // Convert RayShape device type to MNN backend config
            static ErrorCode ConvertFromDevice(const DeviceType &src,
                                               MNN::BackendConfig &backend_config,
                                               MNNForwardType &forward_type);

            // Convert data format from string to RayShape data format
            static DataFormat ConvertToDataFormat(const std::string &src);

            // Convert RayShape data format to MNN tensor format
            static ErrorCode ConvertFromDataFormat(MNN::Tensor::DimensionType &dst,
                                                   const DataFormat src);

            // Get default session config
            static ErrorCode GetDefaultSessionConfig(MNN::ScheduleConfig &config);

            // Convert from session config map
            static ErrorCode ConvertFromSessionConfig(
                MNN::ScheduleConfig &dst, const std::map<std::string, std::string> &src);

            // Convert tensor info
            static ErrorCode ConvertTensorInfo(MNN::Tensor *mnn_tensor, Blob *rayshape_blob);
        };
    } // namespace mnn
} // namespace rayshape

#endif
