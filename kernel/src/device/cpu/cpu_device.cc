#include "device/cpu/cpu_device.h"

namespace rayshape
{
    namespace device
    {
        TypeDeviceRegister<CpuDevice> g_cpu_device_register(DEVICE_TYPE_CPU);

        CpuDevice::CpuDevice(DeviceType device_type) : AbstractDevice(device_type) // 已有主体的报错是在声明时候就定义了
        {
        }

        CpuDevice::~CpuDevice()
        {
        }

        ErrorCode CpuDevice::Allocate(size_t size, void **handle)
        {
            ErrorCode ret = RS_SUCCESS;

            if (handle == nullptr)
            {
                // log
                return RS_INVALID_PARAM;
            }

            if (size > 0)
            {
                *handle = malloc(size);
                if (*handle && size > 0)
                {
                    memset(*handle, 0, size);
                }
            }
            else if (size == 0)
            {
                // support empty blob for yolov5 Slice_507, only in device cpu
                *handle = NULL;
            }
            else
            {
                // log
                return RS_INVALID_PARAM;
            }
            return ret;
        }

        ErrorCode CpuDevice::Free(void *handle)
        {
            if (handle)
            {
                free(handle);
            }

            return RS_SUCCESS;
        }

        ErrorCode CpuDevice::Copy(void *src, void *dst, size_t size,
                                  void *command_queue)
        {

            return RS_SUCCESS;
        }

        ErrorCode CpuDevice::CopyToDevice(void *src, void *dst, size_t size)
        {

            return RS_SUCCESS;
        }

        ErrorCode CpuDevice::CopyFromDevice(void *src, void *dst, size_t size)
        {

            return RS_SUCCESS;
        }

        ErrorCode CpuDevice::Copy(Buffer *dst, const Buffer *src, void *command_queue)
        {

            return RS_SUCCESS;
        }

        ErrorCode CpuDevice::CopyToDevice(Buffer *dst, const Buffer *src, void *command_queue)
        {

            return RS_SUCCESS;
        }

        ErrorCode CpuDevice::CopyFromDevice(Buffer *dst, const Buffer *src, void *command_queue)
        {

            return RS_SUCCESS;
        }

    }
}