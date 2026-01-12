#ifndef _TENSORRT_NETWORK_H_
#define _TENSORRT_NETWORK_H_

#include "inference/inference.h"
#include "tensorrt_include.h"
#include "inference/tensorrt/tensorrt_common.h"

namespace rayshape
{
    namespace inference
    {

        class TensorRTNetWork: public Inference {
        public:
            TensorRTNetWork(InferenceType type);
            ~TensorRTNetWork() override;

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
            ErrorCode CreateOrUpdateBlob(Blob **dst, const char *blob_name);
            ErrorCode CreateBlobArray();
            void ClearBlobArray();

        private:
            // static std::mutex g_mutex;
            DeviceType device_type_ = DeviceType::NONE;
            int num_threads_ = 4;

            tensorrt::TrtUniquePtr<nvinfer1::IRuntime> runtime_ = nullptr;
            tensorrt::TrtUniquePtr<nvinfer1::ICudaEngine> engine_ = nullptr;
            tensorrt::TrtUniquePtr<nvinfer1::IExecutionContext> exec_ctx_ = nullptr;
#if NV_TENSORRT_MAJOR <= 7
            std::vector<void *> bindings_;
#endif

            std::map<std::string, Dims> input_min_shapes_;
            std::map<std::string, Dims> input_max_shapes_;

            Blob **input_blob_arr_ = nullptr;
            size_t input_blob_size_ = 0;

            Blob **output_blob_arr_ = nullptr;
            size_t output_blob_size_ = 0;
        };
    } // namespace inference
} // namespace rayshape

#endif