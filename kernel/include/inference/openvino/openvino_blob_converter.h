#ifndef _OPENVINO_BLOB_CONVERTER_H_
#define _OPENVINO_BLOB_CONVERTER_H_

#include "memory_manager/blob.h"
#include "base/error.h"
#include "openvino_include.h"

namespace rayshape
{
    namespace openvino
    {

        class OpenvinoBlobConverter
        {
        public:
            /**
             *    @brief InferenceEngine::Blob::Ptr 转换为 自有Blob
             *
             *    浅拷贝,src的生存周期必须比dst长,当前支持cpu、x86、arm
             *     @param[in]  src  InferenceEngine::Blob::Ptr
             *     @param[out] dst  Blob**
             *     @return ErrorCode
             */
            static ErrorCode CreateOrUpdateBlob(Blob **dst, const ov::Tensor &src, const char *blob_name, bool alloc, bool gpu_blob = false);

            /**
             *    @brief 自有Blob 转换为 InferenceEngine::Blob::Ptr
             *
             *     浅拷贝,src的生存周期必须比dst长
             *     @param[in]  src  Blob*
             *     @return std::shared_ptr<ov::Tensor>
             */
            static std::shared_ptr<ov::Tensor> ConvertFromBlob(ErrorCode &status, const Blob *src);
        };

    }
} // namespace rayshape::openvino

#endif