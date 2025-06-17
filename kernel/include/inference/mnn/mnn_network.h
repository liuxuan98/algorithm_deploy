#ifndef _MNN_NETWORK_H_
#define _MNN_NETWORK_H_

#include "utils/model.h"
#include "base/common.h"
#include "base/error.h"
#include "base/macros.h"
#include "mnn_include.h"
#include "memory_manager/blob.h"
#include "inference/inference.h"

namespace rayshape
{
    namespace inference
    {

        class MnnNetwork: public Inference {
        public:
            MnnNetwork(InferenceType type);
            virtual ~MnnNetwork();

            virtual ErrorCode Init(const Model *model, const CustomRuntimeV2 *runtime);
            virtual void DeInit();

            virtual ErrorCode Reshape(const char **name_arr, const Dims *dims_arr,
                                      size_t dims_size);
            virtual ErrorCode Forward();
            virtual ErrorCode InputBlobsGet(const Blob ***blob_arr, size_t *blob_size);
            virtual ErrorCode OutputBlobsGet(const Blob ***blob_arr, size_t *blob_size);
            virtual ErrorCode InputBlobGet(const char *input_name, Blob **blob);
            virtual ErrorCode OutputBlobGet(const char *output_name, const Blob **blob);
            // 如果有需要动态转换指针调用改接口
            ErrorCode AddOutput(const std::string &output_name);

        private:
            ErrorCode Reshape();
            ErrorCode InitWithJson(Model model, const CustomRuntimeV2 *runtime,
                                   const JsonHandle json_handle);

            ErrorCode ParseInputShapes(const JsonHandle json_handle); // rapid_json 三方库解析

            ErrorCode CreateBlobArray();
            ErrorCode CreateInferRequest();
            void ClearBlobArray();

        private:
            MNN::Interpreter *interpreter_ = nullptr;
            MNN::Session *session_ = nullptr;
            MagicXEDeviceTypeV2 device_type_;

            std::map<std::string, Dims> input_max_shapes_;
            std::map<std::string, Dims> input_min_shapes_;

            Blob **input_blob_arr_ = nullptr;
            size_t input_blob_size_ = 0;

            Blob **output_blob_arr_ = nullptr;
            size_t output_blob_size_ = 0;

            std::vector<std::string> save_tensors_;

            bool gpu_blob_ = false;
        };

    } // namespace inference
} // namespace rayshape

#endif // MNN_NETWORK_H_
