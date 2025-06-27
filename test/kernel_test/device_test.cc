#include "gtest/gtest.h"
#include "device/cpu/cpu_device.h"
#include "memory_manager/buffer.h"

using namespace rayshape;
using namespace rayshape::device; 
using namespace rayshape::device;

TEST(CpuDeviceTest, AllocateAndFreeMemory_Successfully)
{
    CpuDevice device(DEVICE_TYPE_X86);

    void *ptr = nullptr;
    size_t size = 1024;

    EXPECT_EQ(device.Allocate(size, &ptr), RS_SUCCESS);
    EXPECT_NE(ptr, nullptr);
    //EXPECT_EQ(device.GetLastAllocatedSize(), size);

    EXPECT_EQ(device.Free(ptr), RS_SUCCESS);
}

TEST(CpuDeviceTest, CopyMemory_BufferCopy_ReturnSuccess)
{
    CpuDevice device(DEVICE_TYPE_X86);

    const size_t size = 1024;
    char src_data[size] = "Hello World!";
    char dst_data[size] = {0};

    //Buffer src{src_data, size};
    //Buffer dst{dst_data, size};

    //EXPECT_EQ(device.Copy(&dst, &src, nullptr), RS_SUCCESS);
    //EXPECT_EQ(memcmp(dst.data, src.data, size), 0);
}

TEST(CpuDeviceTest, CopyMemory_PointerCopy_ReturnSuccess)
{
    CpuDevice device(DEVICE_TYPE_X86);

    const size_t size = 1024;
    char src_data[size] = "Hello World!";
    char dst_data[size] = {0};

    EXPECT_EQ(device.Copy(src_data, dst_data, size, nullptr), RS_SUCCESS);
    EXPECT_EQ(memcmp(src_data, dst_data, size), 0);
}

//TEST(CpuDeviceTest, Free_InvalidPointer_ReturnError)
//{
//    CpuDevice device(DEVICE_TYPE_X86);
//
//    void *ptr = nullptr;
//    EXPECT_EQ(device.Free(&ptr), RS_INVALID_PARAM);
//}