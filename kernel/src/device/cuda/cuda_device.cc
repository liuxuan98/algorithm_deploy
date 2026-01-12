#include "device/cuda/cuda_device.h"

#define CHECK_CUDA_RET(expr, name, size, ec)                                                       \
    if (expr != cudaSuccess) {                                                                     \
        RS_LOGE("ERROR: %s failed, size = %zu bytes.\n", name, size);                              \
        return ec;                                                                                 \
    }

namespace rayshape
{
    namespace device
    {
        ErrorCode GetMemcpyKind(const Buffer *src, const Buffer *dst, cudaMemcpyKind &kind) {
            MemoryType src_mem_type = src->GetMemoryType();
            MemoryType dst_mem_type = dst->GetMemoryType();

            if (src_mem_type == MemoryType::CUDA) {
                if (dst_mem_type == MemoryType::HOST) {
                    kind = cudaMemcpyKind::cudaMemcpyDeviceToHost;
                    return ErrorCode::RS_SUCCESS;
                } else if (dst_mem_type == MemoryType::CUDA) {
                    kind = cudaMemcpyKind::cudaMemcpyDeviceToDevice;
                    return ErrorCode::RS_SUCCESS;
                }
            } else if (src_mem_type == MemoryType::HOST) {
                if (dst_mem_type == MemoryType::HOST) {
                    kind = cudaMemcpyKind::cudaMemcpyHostToHost;
                    return ErrorCode::RS_SUCCESS;
                } else if (dst_mem_type == MemoryType::CUDA) {
                    kind = cudaMemcpyKind::cudaMemcpyHostToDevice;
                    return ErrorCode::RS_SUCCESS;
                }
            }

            RS_LOGE("Unsupport memcpy: %d -> %d", src_mem_type, dst_mem_type);
            return ErrorCode::RS_INVALID_PARAM;
        }

        TypeDeviceRegister<CudaDevice> g_cuda_device_register(DeviceType::CUDA);
        // 已有主体的报错是在声明时候就定义了
        CudaDevice::CudaDevice(DeviceType device_type) : AbstractDevice(device_type) {}

        CudaDevice::~CudaDevice() = default;

        ErrorCode CudaDevice::Allocate(size_t size, void **handle) {
            if (handle == nullptr) {
                RS_LOGE("handle is null:%p\n", handle);
                return RS_INVALID_PARAM;
            }

            if (size > 0) {
                CHECK_CUDA_RET(cudaMalloc(handle, size), "cudaMalloc", size,
                               ErrorCode::RS_CUDA_MALLOC_ERROR)
                if (*handle == nullptr) {
                    RS_LOGE("Cuda malloc failed!\n");
                    return RS_OUTOFMEMORY;
                }
                CHECK_CUDA_RET(cudaMemset(*handle, 0, size), "cudaMemset", size,
                               ErrorCode::RS_CUDA_MEMSET_ERROR)
            } else {
                RS_LOGE("Cuda Allocate size:%zu is less than one byte.\n", size);
                return RS_INVALID_PARAM;
            }
            return RS_SUCCESS;
        }

        ErrorCode CudaDevice::Free(void *handle) {
            if (handle == nullptr) {
                RS_LOGI("Cuda free handle ptr is null:%p\n", handle);
                return RS_INVALID_PARAM;
            }
            CHECK_CUDA_RET(cudaFree(handle), "cudaFree", 0UL, ErrorCode::RS_CUDA_FREE_ERROR)
            return RS_SUCCESS;
        }

        ErrorCode CudaDevice::Copy(const void *src, void *dst, size_t size, void *command_queue) {
            if (src == nullptr || dst == nullptr || size == 0) {
                RS_LOGE("Invalid parameters: src=%p, dst=%p, size=%zu", src, dst, size);
                return RS_INVALID_PARAM;
            }
            CHECK_CUDA_RET(cudaMemcpy(dst, src, size, cudaMemcpyKind::cudaMemcpyDeviceToDevice),
                           "cudaMemcpy device to device", size, ErrorCode::RS_CUDA_MEMCPY_ERROR)
            return RS_SUCCESS;
        }

        ErrorCode CudaDevice::CopyToDevice(const void *src, void *dst, size_t size,
                                           void *command_queue) {
            if (src == nullptr || dst == nullptr || size == 0) {
                RS_LOGE("Invalid parameters: src=%p, dst=%p, size=%zu", src, dst, size);
                return RS_INVALID_PARAM;
            }
            CHECK_CUDA_RET(cudaMemcpy(dst, src, size, cudaMemcpyKind::cudaMemcpyHostToDevice),
                           "cudaMemcpy host to device", size, ErrorCode::RS_CUDA_MEMCPY_ERROR)

            return RS_SUCCESS;
        }

        ErrorCode CudaDevice::CopyFromDevice(const void *src, void *dst, size_t size,
                                             void *command_queue) {
            if (src == nullptr || dst == nullptr || size == 0) {
                RS_LOGE("Invalid parameters: src=%p, dst=%p, size=%zu", src, dst, size);
                return RS_INVALID_PARAM;
            }
            CHECK_CUDA_RET(cudaMemcpy(dst, src, size, cudaMemcpyKind::cudaMemcpyDeviceToHost),
                           "cudaMemcpy device to host", size, ErrorCode::RS_CUDA_MEMCPY_ERROR)
            return RS_SUCCESS;
        }

        ErrorCode CudaDevice::Copy(const Buffer *src, Buffer *dst, void *command_queue) {
            if (dst == nullptr || src == nullptr) {
                RS_LOGE("Invalid parameters: dst=%p, src=%p\n", dst, src);
                return RS_INVALID_PARAM;
            }
            RSMemoryInfo dst_mem_info = dst->GetMemoryInfo();
            RSMemoryInfo src_mem_info = src->GetMemoryInfo();
            if (dst_mem_info.data_type_ != src_mem_info.data_type_) {
                RS_LOGE("Buffer copy not support cross data_type:(%d,%d)\n",
                        dst_mem_info.data_type_, src_mem_info.data_type_);
                return RS_INVALID_PARAM;
            }

            size_t dst_size = dst->GetDataSize();
            size_t src_size = src->GetDataSize();
            size_t size = std::min(dst_size, src_size) * GetBytesSize(dst_mem_info.data_type_);

            if (dst->GetDataPtr() == nullptr || src->GetDataPtr() == nullptr) {
                RS_LOGE("dst pointer or src pointer is null\n");
                return RS_INVALID_PARAM;
            }
            if (dst->GetDataPtr() != src->GetDataPtr()) {
                cudaMemcpyKind kind;
                ErrorCode err = GetMemcpyKind(src, dst, kind);
                if (err != ErrorCode::RS_SUCCESS) {
                    return err;
                }
                CHECK_CUDA_RET(cudaMemcpy(dst->GetDataPtr(), src->GetDataPtr(), size, kind),
                               "cudaMemcpy host to device", size, ErrorCode::RS_CUDA_MEMCPY_ERROR)
            }

            return RS_SUCCESS;
        }

        ErrorCode CudaDevice::CopyToDevice(const Buffer *src, Buffer *dst, void *command_queue) {
            if (dst == nullptr || src == nullptr) {
                RS_LOGE("Invalid parameters: dst=%p, src=%p\n", dst, src);
                return RS_INVALID_PARAM;
            }
            RSMemoryInfo dst_mem_info = dst->GetMemoryInfo();
            RSMemoryInfo src_mem_info = src->GetMemoryInfo();
            if (dst_mem_info.data_type_ != src_mem_info.data_type_
                || dst_mem_info.mem_type_ != MemoryType::CUDA
                || src_mem_info.mem_type_ != MemoryType::HOST) {
                RS_LOGE("Buffer copy not support cross data_type:(%d,%d) or men_type:(%d,%d) \n",
                        dst_mem_info.data_type_, src_mem_info.data_type_, dst_mem_info.mem_type_,
                        src_mem_info.mem_type_);
                return RS_INVALID_PARAM;
            }

            size_t dst_size = dst->GetDataSize();
            size_t src_size = src->GetDataSize();
            size_t size = std::min(dst_size, src_size) * GetBytesSize(dst_mem_info.data_type_);

            if (dst->GetDataPtr() == nullptr || src->GetDataPtr() == nullptr) {
                RS_LOGE("dst pointer or src pointer is null\n");
                return RS_INVALID_PARAM;
            }
            if (dst->GetDataPtr() != src->GetDataPtr()) {
                CopyToDevice(src->GetDataPtr(), dst->GetDataPtr(), size, command_queue);
            }

            return RS_SUCCESS;
        }

        ErrorCode CudaDevice::CopyFromDevice(const Buffer *src, Buffer *dst, void *command_queue) {
            if (dst == nullptr || src == nullptr) {
                RS_LOGE("Invalid parameters: dst=%p, src=%p\n", dst, src);
                return RS_INVALID_PARAM;
            }
            RSMemoryInfo dst_mem_info = dst->GetMemoryInfo();
            RSMemoryInfo src_mem_info = src->GetMemoryInfo();
            if (dst_mem_info.data_type_ != src_mem_info.data_type_
                || dst_mem_info.mem_type_ != MemoryType::HOST
                || src_mem_info.mem_type_ != MemoryType::CUDA) {
                RS_LOGE("Buffer copy not support cross data_type:(%d,%d) or men_type:(%d,%d) \n",
                        dst_mem_info.data_type_, src_mem_info.data_type_, dst_mem_info.mem_type_,
                        src_mem_info.mem_type_);
                return RS_INVALID_PARAM;
            }

            size_t dst_size = dst->GetDataSize();
            size_t src_size = src->GetDataSize();
            size_t size = std::min(dst_size, src_size) * GetBytesSize(dst_mem_info.data_type_);

            if (dst->GetDataPtr() == nullptr || src->GetDataPtr() == nullptr) {
                RS_LOGE("dst pointer or src pointer is null\n");
                return RS_INVALID_PARAM;
            }
            if (dst->GetDataPtr() != src->GetDataPtr()) {
                CopyFromDevice(src->GetDataPtr(), dst->GetDataPtr(), size, command_queue);
            }

            return RS_SUCCESS;
        }
    } // namespace device
} // namespace rayshape
