#include "gtest/gtest.h"
#include "device/abstract_device.h"
#include "device/cpu/cpu_device.h"
#include "memory_manager/buffer.h"
#include <omp.h>
#include <algorithm>

using namespace rayshape;
using namespace rayshape::device;

TEST(CpuDeviceTest, AllocTest) {
    DeviceType device_type = DeviceType::CPU;
    AbstractDevice *device_obj_cpu = GetDevice(device_type);
    EXPECT_TRUE(device_obj_cpu);

    size_t alloc_size = 1000000; // btye_size
    void *mem_ptr = nullptr;

    ErrorCode ret = device_obj_cpu->Allocate(0, &mem_ptr);
    EXPECT_EQ(ret, RS_INVALID_PARAM);
    ret = RS_SUCCESS;
    ret = device_obj_cpu->Allocate(alloc_size, nullptr);
    EXPECT_EQ(ret, RS_INVALID_PARAM);

    ret = device_obj_cpu->Allocate(alloc_size, &mem_ptr);
    EXPECT_EQ(ret, RS_SUCCESS);
    EXPECT_TRUE(mem_ptr);

    EXPECT_EQ(device_obj_cpu->Free(mem_ptr), RS_SUCCESS);
    if (mem_ptr != nullptr) {
        mem_ptr = nullptr;
    }
    EXPECT_EQ(device_obj_cpu->Free(nullptr), RS_INVALID_PARAM);

    device_type = DeviceType::X86;
    AbstractDevice *device_obj_x86 = GetDevice(device_type);
    EXPECT_FALSE(device_obj_x86);
}

TEST(CpuDeviceTest, DeviceCopyTest) {

    DeviceType device_type = DeviceType::CPU;
    AbstractDevice *device_obj_cpu = GetDevice(device_type);
    EXPECT_TRUE(device_obj_cpu);

    auto ret = device_obj_cpu->CopyToDevice(nullptr,nullptr,0,nullptr);
    EXPECT_EQ(ret,RS_NOT_IMPLEMENT);

    ret = device_obj_cpu->CopyFromDevice(nullptr,nullptr,0,nullptr);
    EXPECT_EQ(ret, RS_NOT_IMPLEMENT);
}

TEST(CpuDeviceTest, CopyTest) {

    DeviceType device_type = DeviceType::CPU;
    AbstractDevice *device_obj_cpu = GetDevice(device_type);
    EXPECT_TRUE(device_obj_cpu);

    void *src_ptr = nullptr;
    int src_size = 100 * sizeof(float); // byte_size
    ErrorCode ret = device_obj_cpu->Allocate(src_size, &src_ptr);
    EXPECT_EQ(ret, RS_SUCCESS);
    int cout = src_size / sizeof(float);

    float *float_ptr = (float *)src_ptr;
#pragma omp parallel for
    for ( int i = 0; i < cout; i++) {
        float_ptr[i] = float(i);
        //printf("OpenMP Test, th_id: %d\n", omp_get_thread_num());
    }
    int dst_size = 90 * sizeof(float);
    void *dst_ptr = nullptr;
    ret = device_obj_cpu->Allocate(dst_size, &dst_ptr);
    EXPECT_EQ(ret, RS_SUCCESS);
    int copy_size = std::min(src_size, dst_size);
    ret = device_obj_cpu->Copy(src_ptr,dst_ptr, copy_size,nullptr);
    EXPECT_EQ(ret, RS_SUCCESS);

    float *dst_float_ptr = (float *)dst_ptr;
    int element_num = dst_size / sizeof(float);
#pragma omp parallel for
    for (int i = 0; i < element_num; i++) {
        EXPECT_EQ(float_ptr[i], dst_float_ptr[i]);
    }

    device_obj_cpu->Free(src_ptr);
    src_ptr = nullptr;

    device_obj_cpu->Free(dst_ptr);
    dst_ptr = nullptr;

}

TEST(CpuDeviceTest, BufferCopyTest) {

    DeviceType device_type = DeviceType::CPU;
    AbstractDevice *device_obj_cpu = GetDevice(device_type);
    EXPECT_TRUE(device_obj_cpu);

    //usually
    RSMemoryInfo mem_info = {MemoryType::HOST, DataType::FLOAT, 100};
    Buffer *src_buffer = Buffer::Alloc(mem_info);
    float *src_element = (float *)RSBufferDataGet(src_buffer);
    for (int i = 0; i < mem_info.size_; i++) {
        src_element[i] = i;
    }

    Buffer *dst_buffer_v1 = nullptr;

    auto ret = device_obj_cpu->Copy(src_buffer,dst_buffer_v1,nullptr);
    EXPECT_EQ(ret, RS_INVALID_PARAM);

    RSMemoryInfo mem_info_v2 = {MemoryType::CUDA, DataType::FLOAT, 120};
    Buffer *dst_buffer_v2 = Buffer::Alloc(mem_info_v2);
    ret = device_obj_cpu->Copy(src_buffer, dst_buffer_v2, nullptr); // same mem type
    EXPECT_EQ(ret, RS_INVALID_PARAM);

    RSMemoryInfo mem_info_v3 = {MemoryType::HOST, DataType::FLOAT, 120};
    Buffer *dst_buffer_v3 = Buffer::Alloc(mem_info_v3);
    ret = device_obj_cpu->Copy(src_buffer, dst_buffer_v3, nullptr); // same mem type
    EXPECT_EQ(ret, RS_SUCCESS);

    float *dst_element = (float *)RSBufferDataGet(dst_buffer_v3);
    for (int i = 0; i < mem_info.size_; i++) {
        EXPECT_EQ(src_element[i], dst_element[i]);
    }

    RSMemoryInfo mem_info_v4 = {MemoryType::HOST, DataType::UINT8, 120};
    Buffer *dst_buffer_v4 = Buffer::Alloc(mem_info_v4);
    ret = device_obj_cpu->Copy(src_buffer, dst_buffer_v4, nullptr); // same mem type
    EXPECT_EQ(ret, RS_INVALID_PARAM);

    delete src_buffer;
    delete dst_buffer_v2;
    delete dst_buffer_v3;
    delete dst_buffer_v4;
    
}