#ifndef OPENVINO_NETWORK_H
#define OPENVINO_NETWORK_H

#include "inference/inference.h"
#include "openvino_include.h"
#include "utils/json_utils.h"

using namespace rayshape::utils;

// typedef void *JsonHandle;

namespace rayshape
{
    namespace inference
    {

        class OpenVinoNetWork: public Inference {
        public:
            OpenVinoNetWork(InferenceType type);
            ~OpenVinoNetWork() override;

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
            // ErrorCode WriteXmlFile(const std::string &utf8_file_path, const std::string
            // &xml_file_path,std::string bin_data);

            ErrorCode InitWithXml(const std::string &xml_path, const std::string &bin_path);

            ErrorCode InitWithMemoryContent(const std::string &xml_content,
                                            const std::string &bin_content);

            // ErrorCode InitWithJson(const Model* model, const CustomRuntime *runtime,
            //                        const RSJsonHandle json_handle);

            // ErrorCode ParseInputShapes(const JsonObject shapes_obj, std::map<std::string, Dims *>
            // &input_shapes);
            ErrorCode ParseInputShapes(const RSJsonHandle json_handle);
            // 通过json文件的内容读取

            void ClearBlobArray();

            ErrorCode CreateBlobArray();

            ErrorCode Reshape();

        private:
            // static std::mutex g_mutex;
            DeviceType device_type_ = DeviceType::NONE;
            int num_threads_ = 4;

            // std::string model_bin_content_;

            std::shared_ptr<ov::Core> core_ = nullptr;
            std::shared_ptr<ov::Model> model_ = nullptr;

            ov::CompiledModel compiled_model_;
            ov::InferRequest infer_request_;

            std::map<std::string, Dims> input_min_shapes_;
            std::map<std::string, Dims> input_max_shapes_;

            Blob **input_blob_arr_ = nullptr;
            size_t input_blob_size_ = 0;

            Blob **output_blob_arr_ = nullptr;
            size_t output_blob_size_ = 0;
        };
    } // namespace inference
} // namespace rayshape

#endif // OPENVINO_NETWORK_H
