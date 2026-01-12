#include "device/cpu/cpu_device.h"

namespace rayshape
{
    namespace device
    {
        TypeDeviceRegister<CpuDevice> g_cpu_device_register(DeviceType::CPU);
        // 已有主体的报错是在声明时候就定义了
        CpuDevice::CpuDevice(DeviceType device_type) : AbstractDevice(device_type) {}

        CpuDevice::~CpuDevice() = default;

        ErrorCode CpuDevice::Allocate(size_t size, void **handle) {
            if (handle == nullptr) {
                RS_LOGE("handle is null:%p\n", handle);
                return RS_INVALID_PARAM;
            }

            if (size > 0) {
                void *mem_ptr = malloc(size);
                if (mem_ptr == nullptr) {
                    RS_LOGE("Cpu malloc failed!\n");
                    return RS_OUTOFMEMORY;
                }
                memset(mem_ptr, 0, size);
                *handle = mem_ptr;
            } else {
                RS_LOGE("Cpu Allocate size:%zu is less than one byte.\n", size);
                return RS_INVALID_PARAM;
            }
            return RS_SUCCESS;
        }

        ErrorCode CpuDevice::Free(void *handle) {
            if (handle == nullptr) {
                RS_LOGI("Cpu free handle ptr is null:%p\n", handle);
                return RS_INVALID_PARAM;
            }

            free(handle);

            return RS_SUCCESS;
        }

        ErrorCode CpuDevice::Copy(const void *src, void *dst, size_t size, void *command_queue) {
            if (src == nullptr || dst == nullptr || size == 0) {
                RS_LOGE("Invalid parameters: src=%p, dst=%p, size=%zu", src, dst, size);
                return RS_INVALID_PARAM;
            }
            memcpy(dst, src, size);
            // RS_LOGI("CPU Copy %zu bytes from %p to %p", size, src, dst);

            return RS_SUCCESS;
        }

        ErrorCode CpuDevice::CopyToDevice(const void *src, void *dst, size_t size,
                                          void *command_queue) {
            RS_LOGI("CPU CopyToDevice is not implement\n");
            return RS_NOT_IMPLEMENT;
        }

        ErrorCode CpuDevice::CopyFromDevice(const void *src, void *dst, size_t size,
                                            void *command_queue) {
            RS_LOGI("CPU CopyFromDevice is not implement\n");
            return RS_NOT_IMPLEMENT;
        }

        ErrorCode CpuDevice::Copy(const Buffer *src, Buffer *dst, void *command_queue) {
            if (dst == nullptr || src == nullptr) {
                RS_LOGE("Invalid parameters: dst=%p, src=%p\n", dst, src);
                return RS_INVALID_PARAM;
            }
            RSMemoryInfo dst_mem_info = dst->GetMemoryInfo();
            RSMemoryInfo src_mem_info = src->GetMemoryInfo();
            if (dst_mem_info.data_type_ != src_mem_info.data_type_
                || dst_mem_info.mem_type_ != src_mem_info.mem_type_) {
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
                memcpy(dst->GetDataPtr(), src->GetDataPtr(), size);
            }
            return RS_SUCCESS;
        }

        ErrorCode CpuDevice::CopyToDevice(const Buffer *src, Buffer *dst, void *command_queue) {
            RS_LOGI("CPU CopyToDevice is not implement\n");
            return RS_NOT_IMPLEMENT;
        }

        ErrorCode CpuDevice::CopyFromDevice(const Buffer *src, Buffer *dst, void *command_queue) {
            RS_LOGI("CPU CopyFromDevice is not implement\n");
            return RS_NOT_IMPLEMENT;
        }
    } // namespace device
} // namespace rayshape
