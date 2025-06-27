#include "memory_manager/buffer.h"
// #include "memory_manager/p_buffer.h"
#include "gtest/gtest.h"

using namespace rayshape;

TEST(KernelBufferTest, DefaultConstructor) {
    Buffer buffer; // default
    unsigned int data_id = buffer.GetDataId();
    EXPECT_EQ(data_id, 0);
    size_t data_size = buffer.GetDataSize();
    EXPECT_EQ(data_size, 0);
    MemoryType buffer_mem_type = buffer.GetMemoryType();
    EXPECT_EQ(buffer_mem_type, MEM_TYPE_NONE);
    void *buffer_data = buffer.GetDataPtr();
    EXPECT_FALSE(buffer_data);
    RSMemoryInfo buffer_mem_info = buffer.GetMemoryInfo();
    EXPECT_EQ(buffer_mem_info.data_type, DATA_TYPE_NONE);
}

TEST(KernelBufferTest, AllocSample) {
    size_t data_size = 0;
    MemoryType mem_type = MEM_TYPE_HOST;
    Buffer *buffer = Buffer::Alloc(data_size, mem_type);
    EXPECT_FALSE(buffer);

    data_size = 512 * 512;
    mem_type = MEM_TYPE_HOST;
    buffer = Buffer::Alloc(data_size, mem_type);
    EXPECT_TRUE(buffer);
    data_size = buffer->GetDataSize();
    EXPECT_EQ(data_size, 512 * 512);
    MemoryType buffer_mem_type = buffer->GetMemoryType();
    EXPECT_EQ(buffer_mem_type, MEM_TYPE_HOST);
    delete buffer;
    buffer = nullptr;
    EXPECT_FALSE(buffer);

    // override
    RSMemoryInfo mem_info = { MEM_TYPE_HOST, DATA_TYPE_FLOAT, 100 };
    buffer = Buffer::Alloc(mem_info);
    EXPECT_TRUE(buffer);
    EXPECT_EQ(mem_info.size, buffer->GetDataSize());
    delete buffer;
    buffer = nullptr;
    EXPECT_FALSE(buffer);

    mem_info = { MEM_TYPE_HOST, DATA_TYPE_HALF, 100 };
    buffer = Buffer::Alloc(mem_info);
    EXPECT_TRUE(buffer);
    EXPECT_EQ(mem_info.men_type, buffer->GetMemoryType());
    delete buffer;
    buffer = nullptr;
    EXPECT_FALSE(buffer);

    mem_info = { MEM_TYPE_HOST, DATA_TYPE_INT32, 100 };
    buffer = Buffer::Alloc(mem_info);
    EXPECT_TRUE(buffer);
    EXPECT_EQ(mem_info.data_type, buffer->GetMemoryInfo().data_type);
    delete buffer;
    buffer = nullptr;
    EXPECT_FALSE(buffer);

    mem_info = { MEM_TYPE_HOST, DATA_TYPE_UINT32, 100 };
    buffer = Buffer::Alloc(mem_info);
    EXPECT_TRUE(buffer);
    EXPECT_EQ(mem_info.size, buffer->GetDataSize());
    delete buffer;
    buffer = nullptr;
    EXPECT_FALSE(buffer);

    mem_info = { MEM_TYPE_HOST, DATA_TYPE_INT64, 100 };
    buffer = Buffer::Alloc(mem_info);
    EXPECT_TRUE(buffer);
    EXPECT_EQ(mem_info.size, buffer->GetDataSize());
    delete buffer;
    buffer = nullptr;
    EXPECT_FALSE(buffer);

    mem_info = { MEM_TYPE_HOST, DATA_TYPE_INT8, 0 };
    buffer = Buffer::Alloc(mem_info);
    EXPECT_FALSE(buffer);

    mem_info = { MEM_TYPE_CUDA, DATA_TYPE_INT8, 100 };
    buffer = Buffer::Alloc(mem_info);
    EXPECT_TRUE(buffer);
    EXPECT_NE(mem_info.men_type, buffer->GetMemoryType());
    delete buffer;
    buffer = nullptr;
    EXPECT_FALSE(buffer);
}

TEST(KernelBufferTest, CreateSample) {
    void *data_ptr = nullptr;
    size_t data_size = 100;
    data_ptr = malloc(data_size);
    MemoryType mem_type = MEM_TYPE_HOST;
    Buffer *buffer = Buffer::Create(data_ptr, data_size, mem_type); // this API easy
    EXPECT_TRUE(buffer);
    EXPECT_EQ(buffer->GetMemoryInfo().data_type, DATA_TYPE_UINT8);
    EXPECT_EQ(buffer->GetDataPtr(), data_ptr);
    EXPECT_EQ(buffer->GetMemoryType(), mem_type);
    free(data_ptr);
    delete buffer;
    buffer = nullptr;
    data_ptr = nullptr;

    buffer = Buffer::Create(data_ptr, data_size, mem_type);
    EXPECT_FALSE(buffer);
    data_ptr = malloc(data_size);
    data_size = 0;
    buffer = Buffer::Create(data_ptr, data_size, mem_type);
    EXPECT_FALSE(buffer);
    free(data_ptr);
    data_ptr = nullptr;

    RSMemoryInfo mem_info = { MEM_TYPE_HOST, DATA_TYPE_FLOAT, 100 };
    data_ptr = malloc(mem_info.size);
    buffer = Buffer::Create(data_ptr, mem_info);
    EXPECT_TRUE(buffer);
    EXPECT_EQ(buffer->GetDataPtr(), data_ptr);
    free(data_ptr);
    data_ptr = nullptr;

    mem_info = { MEM_TYPE_HOST, DATA_TYPE_FLOAT, 100 };
    data_ptr = nullptr;
    buffer = Buffer::Create(data_ptr, mem_info);
    EXPECT_FALSE(buffer);
}

TEST(KernelBufferTest, DeepcopySample) {
    RSMemoryInfo mem_info = { MEM_TYPE_HOST, DATA_TYPE_FLOAT, 100 };
    Buffer *buffer = Buffer::Alloc(mem_info);
    EXPECT_TRUE(buffer);
    EXPECT_EQ(mem_info.size, buffer->GetDataSize());

    Buffer dst_buffer(mem_info);
    auto ret = buffer->DeepCopy(dst_buffer);
    EXPECT_EQ(ret, RS_SUCCESS);
}