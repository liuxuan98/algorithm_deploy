#ifndef _MNN_NETWORK_H_
#define _MNN_NETWORK_H_

#include "inference/inference.h"
#include "mnn_include.h"
#include "utils/json_utils.h"

using namespace rayshape::utils;

namespace rayshape
{
    namespace inference
    {
        class MNNNetwork: public Inference {
        public:
            MNNNetwork(InferenceType type);
            ~MNNNetwork() override;

            // Inference interface implementation
            ErrorCode Init(const Model *model, const CustomRuntime *runtime) override;
            void DeInit() override;

            ErrorCode Reshape(const char **name_arr, const Dims *dims_arr,
                              size_t dims_size) override;
            ErrorCode Forward() override;

            ErrorCode InputBlobsGet(const Blob ***blob_arr, size_t *blob_size) override;
            ErrorCode OutputBlobsGet(const Blob ***blob_arr, size_t *blob_size) override;
            ErrorCode InputBlobGet(const char *input_name, Blob **blob) override;
            ErrorCode OutputBlobGet(const char *output_name, const Blob **blob) override;

        private:
            // Private methods
            ErrorCode InitWithModel(const std::string &model_buf);
            ErrorCode InitWithJson(const Model *model, const CustomRuntime *runtime,
                                   const RSJsonHandle json_handle);
            ErrorCode ParseInputShapes(const RSJsonHandle json_handle);
            ErrorCode CreateBlobArray();
            ErrorCode Reshape();
            void ClearBlobArray();

        private:
            // Device and runtime configuration
            DeviceType device_type_ = DeviceType::NONE;
            int num_threads_ = 4;

            // MNN components
            std::shared_ptr<MNN::Interpreter> interpreter_ = nullptr;
            MNN::Session *session_ = nullptr;
            MNN::ScheduleConfig schedule_config_;

            // Input/Output shape configurations
            std::map<std::string, Dims> input_min_shapes_;
            std::map<std::string, Dims> input_max_shapes_;

            // Blob arrays for input/output
            Blob **input_blob_arr_ = nullptr;
            size_t input_blob_size_ = 0;

            Blob **output_blob_arr_ = nullptr;
            size_t output_blob_size_ = 0;

            // Input/Output tensors
            std::vector<MNN::Tensor *> input_tensors_;
            std::vector<MNN::Tensor *> output_tensors_;
        };
    } // namespace inference
} // namespace rayshape

#endif
