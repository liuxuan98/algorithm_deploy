#include "memory_manager/buffer.h"
#include "gtest/gtest.h"

using namespace rayshape;

TEST(KernelBufferTest, BufferConstruTest) {
    Buffer buffer; // default
    unsigned int data_id = buffer.GetDataId();
    EXPECT_EQ(data_id, 0);
    size_t data_size = buffer.GetDataSize();
    EXPECT_EQ(data_size, 0);
    MemoryType buffer_mem_type = buffer.GetMemoryType();
    EXPECT_EQ(buffer_mem_type, MemoryType::NONE);
    void *buffer_data = buffer.GetDataPtr();
    EXPECT_FALSE(buffer_data);
    RSMemoryInfo buffer_mem_info = buffer.GetMemoryInfo();
    EXPECT_EQ(buffer_mem_info.data_type_, DataType::NONE);
    EXPECT_EQ(buffer_mem_info.mem_type_, MemoryType::NONE);
}

TEST(KernelBufferTest, BufferAllocTest) {
    size_t data_size = 0;
    MemoryType mem_type = MemoryType::HOST;
    Buffer *buffer = Buffer::Alloc(data_size, mem_type);
    EXPECT_FALSE(buffer);

    data_size = 512 * 512;
    mem_type = MemoryType::HOST;
    buffer = Buffer::Alloc(data_size, mem_type);
    EXPECT_TRUE(buffer);
    data_size = buffer->GetDataSize();
    EXPECT_EQ(data_size, 512 * 512);
    MemoryType buffer_mem_type = buffer->GetMemoryType();
    EXPECT_EQ(buffer_mem_type, MemoryType::HOST);
    delete buffer;
    buffer = nullptr;
    EXPECT_FALSE(buffer);

    // override
    RSMemoryInfo mem_info = {MemoryType::HOST, DataType::FLOAT, 100};
    buffer = Buffer::Alloc(mem_info);
    EXPECT_TRUE(buffer);
    EXPECT_EQ(mem_info.size_, buffer->GetDataSize());
    delete buffer;
    buffer = nullptr;
    EXPECT_FALSE(buffer);

    mem_info = {MemoryType::HOST, DataType::HALF, 100};
    buffer = Buffer::Alloc(mem_info);
    EXPECT_TRUE(buffer);
    EXPECT_EQ(mem_info.mem_type_, buffer->GetMemoryType());
    delete buffer;
    buffer = nullptr;
    EXPECT_FALSE(buffer);

    mem_info = {MemoryType::HOST, DataType::INT32, 100};
    buffer = Buffer::Alloc(mem_info);
    EXPECT_TRUE(buffer);
    EXPECT_EQ(mem_info.data_type_, buffer->GetMemoryInfo().data_type_);
    delete buffer;
    buffer = nullptr;
    EXPECT_FALSE(buffer);

    mem_info = {MemoryType::HOST, DataType::UINT32, 100};
    buffer = Buffer::Alloc(mem_info);
    EXPECT_TRUE(buffer);
    EXPECT_EQ(mem_info.size_, buffer->GetDataSize());
    delete buffer;
    buffer = nullptr;
    EXPECT_FALSE(buffer);

    mem_info = {MemoryType::HOST, DataType::INT64, 100};
    buffer = Buffer::Alloc(mem_info);
    EXPECT_TRUE(buffer);
    EXPECT_EQ(mem_info.size_, buffer->GetDataSize());
    delete buffer;
    buffer = nullptr;
    EXPECT_FALSE(buffer);

    mem_info = {MemoryType::HOST, DataType::INT8, 0};
    buffer = Buffer::Alloc(mem_info);
    EXPECT_FALSE(buffer);

    mem_info = {MemoryType::CUDA, DataType::INT8, 100};
    buffer = Buffer::Alloc(mem_info);
    EXPECT_TRUE(buffer);
    EXPECT_NE(mem_info.mem_type_, buffer->GetMemoryType());
    delete buffer;
    buffer = nullptr;
    EXPECT_FALSE(buffer);
}

TEST(KernelBufferTest, BufferCreateTest) {
    void *data_ptr = nullptr;
    size_t data_size = 100;
    data_ptr = malloc(data_size);
    MemoryType mem_type = MemoryType::HOST;
    Buffer *buffer = Buffer::Create(data_ptr, data_size, mem_type); // this API easy
    EXPECT_TRUE(buffer);
    EXPECT_EQ(buffer->GetMemoryInfo().data_type_, DataType::UINT8);
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

    RSMemoryInfo mem_info = {MemoryType::HOST, DataType::FLOAT, 100};
    data_ptr = malloc(mem_info.size_);
    buffer = Buffer::Create(data_ptr, mem_info);
    EXPECT_TRUE(buffer);
    EXPECT_EQ(buffer->GetDataPtr(), data_ptr);
    free(data_ptr);
    data_ptr = nullptr;

    mem_info = {MemoryType::HOST, DataType::FLOAT, 100};
    data_ptr = nullptr;
    buffer = Buffer::Create(data_ptr, mem_info);
    EXPECT_FALSE(buffer);
}

TEST(KernelBufferTest, BufferDeepCopyTest) {
    RSMemoryInfo mem_info = {MemoryType::HOST, DataType::FLOAT, 100};
    Buffer *src_buffer = Buffer::Alloc(mem_info);
    EXPECT_TRUE(src_buffer);
    EXPECT_EQ(mem_info.size_, src_buffer->GetDataSize());

    Buffer dst_buffer(mem_info);
    auto ret = src_buffer->DeepCopy(dst_buffer);
    EXPECT_EQ(ret, RS_SUCCESS);

    EXPECT_TRUE(dst_buffer.GetDataPtr() != src_buffer->GetDataPtr());

    Buffer dsr_buffer_v2;
    delete src_buffer;
}