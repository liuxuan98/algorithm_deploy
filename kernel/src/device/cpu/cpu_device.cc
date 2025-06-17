#include "device/cpu/cpu_device.h"

namespace rayshape
{
    namespace device
    {
        TypeDeviceRegister<CpuDevice> g_cpu_device_register(DEVICE_TYPE_CPU);
        // 已有主体的报错是在声明时候就定义了
        CpuDevice::CpuDevice(DeviceType device_type) : AbstractDevice(device_type) {}

        CpuDevice::~CpuDevice() {}

        ErrorCode CpuDevice::Allocate(size_t size, void **handle) {
            if (handle == nullptr) {
                RS_LOGE("handle is null:%p\n", handle);
                return RS_INVALID_PARAM;
            }

            if (size > 0) {
                *handle = malloc(size);
                if (*handle == nullptr) {
                    RS_LOGE("CPU malloc failed!\n");
                    return RS_OUTOFMEMORY;
                }
                memset(*handle, 0, size);
            } else {
                // log
                RS_LOGE("CPU Allocate size:%zu is less than zore.\n", size);
                return RS_INVALID_PARAM;
            }
            return RS_SUCCESS;
        }

        ErrorCode CpuDevice::Free(void *handle) {
            if (handle == nullptr) {
                RS_LOGI("CPU free handle ptr is null:%p\n", handle);
                return RS_SUCCESS;
            }

            free(handle);

            return RS_SUCCESS;
        }

        ErrorCode CpuDevice::Copy(void *src, void *dst, size_t size, void *command_queue) {
            if (src == nullptr || dst == nullptr || size == 0) {
                RS_LOGE("Invalid parameters: src=%p, dst=%p, size=%zu", src, dst, size);
                return RS_INVALID_PARAM;
            }
            memcpy(dst, src, size);
            // RS_LOGI("CPU Copy %zu bytes from %p to %p", size, src, dst);

            return RS_SUCCESS;
        }

        ErrorCode CpuDevice::CopyToDevice(void *src, void *dst, size_t size, void *command_queue) {
            RS_LOGI("CPU CopyToDevice is not implement\n");
            return RS_NOT_IMPLEMENT;
        }

        ErrorCode CpuDevice::CopyFromDevice(void *src, void *dst, size_t size,
                                            void *command_queue) {
            RS_LOGI("CPU CopyFromDevice is not implement\n");
            return RS_NOT_IMPLEMENT;
        }

        ErrorCode CpuDevice::Copy(Buffer *dst, const Buffer *src, void *command_queue) {
            if (dst == nullptr || src == nullptr) {
                RS_LOGE("Invalid parameters: dst=%p, src=%p\n", dst, src);
                return RS_INVALID_PARAM;
            }

            size_t dst_size = dst->GetDataSize();
            size_t src_size = src->GetDataSize();
            size_t size = std::min(dst_size, src_size);

            if (dst->GetDataPtr() == nullptr || src->GetDataPtr() == nullptr) {
                RS_LOGE("dst pointer or src pointer is null\n");
                return RS_INVALID_PARAM;
            }
            if (dst->GetDataPtr() != src->GetDataPtr()) {
                memcpy(dst->GetDataPtr(), src->GetDataPtr(), size);
            }
            return RS_SUCCESS;
        }

        ErrorCode CpuDevice::CopyToDevice(Buffer *dst, const Buffer *src, void *command_queue) {
            RS_LOGI("CPU CopyToDevice is not implement\n");
            return RS_NOT_IMPLEMENT;
        }

        ErrorCode CpuDevice::CopyFromDevice(Buffer *dst, const Buffer *src, void *command_queue) {
            RS_LOGI("CPU CopyFromDevice is not implement\n");
            return RS_NOT_IMPLEMENT;
        }

    } // namespace device
} // namespace rayshape