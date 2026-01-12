#include "inference/mnn/mnn_blob_converter.h"
#include "inference/mnn/mnn_config_converter.h"
#include "utils/memory_size_info.h"
#include "utils/blob_utils.h"
#include "base/logger.h"
#include "utils/device_convert_utils.h"

using namespace rayshape::utils;

namespace rayshape
{
    namespace mnn
    {
        ErrorCode MNNBlobConverter::CreateOrUpdateBlob(Blob **dst, const MNN::Tensor *src,
                                                       const char *blob_name, bool alloc,
                                                       bool gpu_blob) {
            // TODO: Implement MNN Tensor to RayShape Blob conversion
            if (dst == nullptr || src == nullptr || blob_name == nullptr) {
                RS_LOGE("input params dst, src or blob_name is null\n");
                return RS_INVALID_PARAM;
            }

            ErrorCode ret = RS_SUCCESS;

            if (*dst != nullptr) {
                free(*dst); // 防止内存泄漏
                *dst = nullptr;
            }

            Blob *blob = (Blob *)malloc(sizeof(Blob));
            if (blob == nullptr) {
                RS_LOGE("blob malloc Blob:%zu failed.\n", sizeof(Blob));
                return RS_OUTOFMEMORY;
            }
            memset(blob, 0, sizeof(Blob));

            // Convert data type
            halide_type_t halide_type = src->getType();
            blob->data_type = MNNConfigConverter::ConvertToDataType(halide_type);

            // Convert shape dims
            std::vector<int> mnn_shape = src->shape();
            ret = MNNConfigConverter::ConvertToDims(blob->dims, mnn_shape);
            if (ret != RS_SUCCESS) {
                RS_LOGE("MNNConfigConverter::ConvertToDims failed:%d.\n", ret);
                free(blob);
                return ret;
            }

            // Convert data layout
            std::string layout = GetDataLayoutString(blob->dims);
            blob->data_format = MNNConfigConverter::ConvertToDataFormat(layout);

            // Device type
            if (gpu_blob) {
                blob->device_type = DeviceType::OPENCL;
            } else {
                blob->device_type = DeviceType::CPU;
            }

            // Blob name
            strncpy(blob->name, blob_name, strlen(blob_name));

            MemoryType mem_type = MemoryType::NONE;
            ret = ConvertDeviceTypeToMemory(blob->device_type, mem_type);
            if (ret != RS_SUCCESS) {
                RS_LOGE("ConvertDeviceTypeToMemory failed\n");
                free(blob);
                return ret;
            }

            if (alloc) {
                size_t size = CalculateDims(blob->dims);
                RSMemoryInfo mem_info{mem_type, blob->data_type, static_cast<unsigned int>(size)};
                blob->buffer = Buffer::Alloc(mem_info);
                if (blob->buffer == nullptr) {
                    RS_LOGE("Buffer Alloc failed\n");
                    free(blob);
                    return RS_OUTOFMEMORY;
                }
            } else {
                void *data = src->host<void>();
                size_t size = src->size();
                RSMemoryInfo mem_info{mem_type, blob->data_type, static_cast<unsigned int>(size)};
                blob->buffer = Buffer::Create(data, mem_info);
                if (blob->buffer == nullptr) {
                    RS_LOGE("Buffer Create failed\n");
                    free(blob);
                    return RS_OUTOFMEMORY;
                }
            }

            *dst = blob;
            return RS_SUCCESS;
        }

        std::shared_ptr<MNN::Tensor> MNNBlobConverter::ConvertFromBlob(ErrorCode &status,
                                                                       const Blob *src) {
            // TODO: Implement RayShape Blob to MNN Tensor conversion
            if (src == nullptr) {
                status = RS_NULL_PARAM;
                return nullptr;
            }

            status = RS_INVALID_MODEL;

            // Convert data type
            halide_type_t halide_type;
            status = MNNConfigConverter::ConvertFromDataType(halide_type, src->data_type);
            if (status != RS_SUCCESS) {
                RS_LOGE("MNNBlobConverter::ConvertFromDataType failed:%d.\n", status);
                return nullptr;
            }

            // Convert dimensions
            std::vector<int> mnn_shape;
            status = MNNConfigConverter::ConvertFromDims(mnn_shape, src->dims);
            if (status != RS_SUCCESS) {
                RS_LOGE("MNNBlobConverter::ConvertFromDims failed:%d.\n", status);
                return nullptr;
            }

            // Convert data format
            MNN::Tensor::DimensionType dimension_type;
            status = MNNConfigConverter::ConvertFromDataFormat(dimension_type, src->data_format);
            if (status != RS_SUCCESS) {
                RS_LOGE("MNNBlobConverter::ConvertFromDataFormat failed:%d.\n", status);
                return nullptr;
            }

            // Get data pointer
            void *data = (src->buffer)->GetDataPtr();
            if (data == nullptr) {
                RS_LOGE("MNNBlobConverter::ConvertFromBlob data is null.\n");
                status = RS_NULL_PARAM;
                return nullptr;
            }

            // Create MNN Tensor
            std::shared_ptr<MNN::Tensor> dst_tensor = nullptr;
            try {
                // Use MNN::Tensor::create static method
                MNN::Tensor *raw_tensor =
                    MNN::Tensor::create(mnn_shape, halide_type, data, dimension_type);
                if (raw_tensor == nullptr) {
                    RS_LOGE("MNNBlobConverter::ConvertFromBlob create tensor failed\n");
                    status = RS_MODEL_ERROR;
                    return nullptr;
                }
                dst_tensor = std::shared_ptr<MNN::Tensor>(
                    raw_tensor, [](MNN::Tensor *t) { MNN::Tensor::destroy(t); });
                status = RS_SUCCESS;
            } catch (const std::exception &e) {
                RS_LOGE("MNNBlobConverter::ConvertFromBlob create tensor failed: %s\n", e.what());
                status = RS_MODEL_ERROR;
                return nullptr;
            }

            return dst_tensor;
        }

    } // namespace mnn
} // namespace rayshape
