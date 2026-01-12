#include "utils/device_convert_utils.h"

namespace rayshape
{
    namespace utils
    {
        ErrorCode ConvertDeviceTypeToMemory(const DeviceType &device_type, MemoryType &mem_type) {
            switch (device_type) {
            case DeviceType::CPU:
            case DeviceType::X86:
            case DeviceType::ARM:
                mem_type = MemoryType::HOST;
                break;

            case DeviceType::CUDA:
                mem_type = MemoryType::CUDA;
                break;

            case DeviceType::OPENCL:
                mem_type = MemoryType::OPENCL;
                break;

            // 可选：其他异构设备映射为最接近的内存类型
            case DeviceType::INTERL_NPU:
            case DeviceType::INTERL_GPU:
                mem_type = MemoryType::NONE; // 或者根据实际平台调整
                break;
            default:
                mem_type = MemoryType::NONE; // 未知设备类型
                break;
            }

            return RS_SUCCESS;
        }

        ErrorCode ConvertMemoryTypeToDevice(const MemoryType &mem_type, DeviceType &device_data) {
            switch (mem_type) {
            case MemoryType::NONE:
                device_data = DeviceType::NONE;
                break;

            case MemoryType::HOST:
                device_data = DeviceType::CPU;
                break;

            case MemoryType::CUDA:
                device_data = DeviceType::CUDA;
                break;
            case MemoryType::OPENCL:
                device_data = DeviceType::OPENCL;
                break;
            default:
                device_data = DeviceType::NONE;
                break;
            }

            return RS_SUCCESS;
        }
    } // namespace utils
} // namespace rayshape
