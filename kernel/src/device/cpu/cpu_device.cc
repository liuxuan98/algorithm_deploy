#include "device/cpu/cpu_device.h"

namespace rayshape
{
    namespace device
    {
        CpuDevice::CpuDevice(DeviceType device_type) : AbstractDevice(device_type)
        {
        }

        CpuDevice::~CpuDevice()
        {
        }

        ErrorCode CpuDevice::Allocate(size_t size, void **ptr)
        {
            ErrorCode ret = RS_SUCCESS;

            if (ptr == nullptr)
            {
                // 日志
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
                // 日志
                return RS_INVALID_PARAM;
            }
            return ret;
        }

        ErrorCode CpuDevice::Free(void handle)
        {
            if (handle)
            {
                free(handle);
            }

            return RS_SUCCESS;
        }

        TypeDeviceRegister<CpuDevice> g_cpu_device_register(DEVICE_TYPE_CPU);

    }
}