#ifndef _MNN_BLOB_CONVERTER_H_
#define _MNN_BLOB_CONVERTER_H_

#include "memory_manager/blob.h"
#include "base/error.h"
#include "mnn_include.h"

namespace rayshape
{
    namespace mnn
    {
        class MNNBlobConverter {
        public:
            /**
             * @brief Create or update RayShape Blob from MNN Tensor
             *
             * Shallow copy, src tensor lifetime must be longer than dst blob
             * Supports CPU, ARM devices
             *
             * @param[out] dst     Target Blob pointer
             * @param[in]  src     Source MNN Tensor
             * @param[in]  blob_name Blob name
             * @param[in]  alloc   Whether to allocate new memory
             * @param[in]  gpu_blob Whether this is a GPU blob
             * @return ErrorCode
             */
            static ErrorCode CreateOrUpdateBlob(Blob **dst, const MNN::Tensor *src,
                                                const char *blob_name, bool alloc,
                                                bool gpu_blob = false);

            /**
             * @brief Convert RayShape Blob to MNN Tensor
             *
             * Shallow copy, src blob lifetime must be longer than dst tensor
             *
             * @param[out] status  Error code output
             * @param[in]  src     Source RayShape Blob
             * @return std::shared_ptr<MNN::Tensor> Created MNN Tensor
             */
            static std::shared_ptr<MNN::Tensor> ConvertFromBlob(ErrorCode &status, const Blob *src);
        };
    } // namespace mnn
} // namespace rayshape

#endif