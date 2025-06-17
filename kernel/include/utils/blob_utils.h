#ifndef _BLOB_UTILS_H_
#define _BLOB_UTILS_H_

#include "memory_manager/blob.h"

// only inner 对外不公开
namespace rayshape
{
    namespace utils
    {
        static Blob *FindBlobAndIndexByName(Blob **blob_array, size_t blobs_size,
                                            const char *blob_name, int *blob_index) {
            if (blob_array == nullptr || blob_name == nullptr || blobs_size == 0) {
                if (blob_index != nullptr) {
                    *blob_index = -1;
                }
                return nullptr;
            }

            for (size_t i = 0; i < blobs_size; ++i) {
                if (blob_array[i] && blob_array[i]->name
                    && strcmp(blob_array[i]->name, blob_name) == 0) {
                    if (blob_index != nullptr) {
                        *blob_index = static_cast<int>(i);
                    }
                    return blob_array[i];
                }
            }

            if (blob_index != nullptr) {
                *blob_index = -1;
            }
            return nullptr;
        }

        static std::string GetDataLayoutString(const Dims &dims) {
            std::string layout;
            if (dims.size == 4) {
                if (dims.value[1] < dims.value[3]) {
                    layout = "NCHW";
                } else if (dims.value[1] >= dims.value[3]) {
                    layout = "NHWC";
                } else if ((dims.value[1] >= dims.value[3]) && dims.value[3] == 4) {
                    layout = "NHWC4";
                } else {
                    layout = "UNKNOWN";
                }
            } else if (dims.size == 2) {
                layout = "NC";
            } else if (dims.size == 5) {
                layout = "NCDHW";
            } else {
                layout = "UNKNOWN";
            }

            return layout;
        }

    } // namespace utils
} // namespace rayshape

#endif
