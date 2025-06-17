#include "utils/device_convert_utils.h"

namespace rayshape
{
    namespace utils
    {
        ErrorCode ConvertDeviceTypeToMemory(const DeviceType &device_type, MemoryType &mem_type) {
            switch (device_type) {
            case DEVICE_TYPE_CPU:
            case DEVICE_TYPE_X86:
            case DEVICE_TYPE_ARM:
                mem_type = MEM_TYPE_HOST;
                break;

            case DEVICE_TYPE_CUDA:
                mem_type = MEM_TYPE_CUDA;
                break;

            case DEVICE_TYPE_OPENCL:
                mem_type = MEM_TYPE_OPENCL;
                break;

            // 可选：其他异构设备映射为最接近的内存类型
            case DEVICE_TYPE_INTERL_NPU:
            case DEVICE_TYPE_INTERL_GPU:
                mem_type = MEM_TYPE_NONE; // 或者根据实际平台调整
                break;
            default:
                mem_type = MEM_TYPE_NONE; // 未知设备类型
                break;
            }

            return RS_SUCCESS;
        }

        ErrorCode ConvertMemoryTypeToDevice(const MemoryType &mem_type, DeviceType &device_data) {
            switch (mem_type) {
            case MEM_TYPE_NONE:
                device_data = DEVICE_TYPE_NONE;
                break;

            case MEM_TYPE_HOST:
                device_data = DEVICE_TYPE_CPU;
                break;

            case MEM_TYPE_CUDA:
                device_data = DEVICE_TYPE_CUDA;
                break;
            case MEM_TYPE_OPENCL:
                device_data = DEVICE_TYPE_OPENCL;
                break;
            default:
                device_data = DEVICE_TYPE_NONE;
                break;
            }

            return RS_SUCCESS;
        }
    } // namespace utils
} // namespace rayshape