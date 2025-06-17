#ifndef _MNN_BLOB_CONVERTER_H_
#define _MNN_BLOB_CONVERTER_H_

#include "memory_manager/blob.h"
#include "base/error.h"
#include "mnn_include.h"

namespace rayshape
{
    namespace mnn
    {

        class MnnBlobConverter {
        public:
            static ErrorCode CreateOrUpdateBlob(Blob **dst, const MNN::Tensor &src,
                                                const char *blob_name, bool alloc,
                                                bool is_gpu_blob);

            static ErrorCode CopyToBlob(MagicXEBlob *src, MNN::Tensor &dst, DataFormat format);

            static MNN::Tensor *ConvertFromBlob(ErrorCode &status, const Blob *src);
        }
    } // namespace mnn
} // namespace rayshape

#endif