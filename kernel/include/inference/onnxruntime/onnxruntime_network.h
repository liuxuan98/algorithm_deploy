#ifndef _ONNXRUNTIME_NETWORK_H_
#define _ONNXRUNTIME_NETWORK_H_

#include "inference/inference.h"
#include "onnxruntime_include.h"

namespace rayshape
{
    namespace inference
    {

        class ONNXRuntimeNetWork: public Inference {
        public:
            ONNXRuntimeNetWork(InferenceType type);
            ~ONNXRuntimeNetWork() override;

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
            ErrorCode CreateOrUpdateBlob(Blob **dst, const char *blob_name, size_t idx,
                                         bool is_input);
            ErrorCode CreateBlobArray();
            void ClearBlobArray();

            // ErrorCode Reshape();

        private:
            // static std::mutex g_mutex;
            DeviceType device_type_ = DeviceType::NONE;
            int num_threads_ = 4;

            std::vector<const char *> input_names_;
            std::vector<const char *> output_names_;
            std::vector<Ort::Value> input_tensors_;
            std::vector<Ort::Value> output_tensors_;

            std::shared_ptr<Ort::Env> env_;
            std::shared_ptr<Ort::Session> session_;

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