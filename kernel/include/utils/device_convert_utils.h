#ifndef _DEVICE_CONVERT_UTILS_H_
#define _DEVICE_CONVERT_UTILS_H_

#include "base/common.h"
#include "base/error.h"

// only inner 对外不公开
namespace rayshape
{
    namespace utils
    {
        ErrorCode ConvertDeviceTypeToMemory(const DeviceType &device_type, MemoryType &mem_type);

        ErrorCode ConvertMemoryTypeToDevice(const MemoryType &mem_type, DeviceType &device_type);

    } // namespace utils
} // namespace rayshape

#endif