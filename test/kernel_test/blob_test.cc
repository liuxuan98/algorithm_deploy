#include "memory_manager/blob.h"
#include "gtest/gtest.h"

using namespace rayshape;
TEST(BlobTest, BlobAllocTest) {
    Dims dims{4, {1, 3, 224, 224}};
    Blob *blob_0 = BlobAlloc(DeviceType::X86, DataType::FLOAT, DataFormat::NCHW, "x86_blob", &dims);
    EXPECT_TRUE(blob_0);
    BlobFree(blob_0);
    blob_0 = nullptr; // set null by manual.

    Blob *blob_1 = BlobAlloc(DeviceType::X86, DataType::FLOAT, DataFormat::NCHW, nullptr, &dims);
    EXPECT_FALSE(blob_1);

    Blob *blob_2 =
        BlobAlloc(DeviceType::X86, DataType::FLOAT, DataFormat::NCHW, "cpu_blob", nullptr);
    EXPECT_FALSE(blob_2);
}

TEST(BlobTest, BlobCopyTest) {
    Dims dims{4, {1, 3, 224, 224}};
    Blob *src = BlobAlloc(DeviceType::CPU, DataType::FLOAT, DataFormat::NCHW, "cpu_blob", &dims);
    Blob *dst =
        BlobAlloc(DeviceType::CPU, DataType::FLOAT, DataFormat::NCHW, "another_cpu_blob", &dims);

    // 填充一些数据
    float *src_data = static_cast<float *>(src->buffer->GetDataPtr());
    for (int i = 0; i < 3 * 224 * 224; ++i) {
        src_data[i] = static_cast<float>(i);
    }

    EXPECT_EQ(BlobCopy(src, dst), RS_SUCCESS);

    float *dst_data = static_cast<float *>(dst->buffer->GetDataPtr());
    for (int i = 0; i < 3 * 224 * 224; ++i) {
        if (static_cast<float>(i) != dst_data[i]) {
            std::cout << "debug" << std::endl;
        }
        EXPECT_EQ(dst_data[i], static_cast<float>(i));
    }

    BlobFree(src);
    BlobFree(dst);


}

TEST(BlobTest, BlobFreeTest) {
    Dims dims{4, {1, 3, 224, 224}};
    Blob *blob = BlobAlloc(DeviceType::X86, DataType::FLOAT, DataFormat::NCHW, "test_blob", &dims);

    EXPECT_NE(blob, nullptr);

    EXPECT_EQ(BlobFree(blob), RS_SUCCESS);

    if (blob != nullptr) {
        blob = nullptr;
    }

    EXPECT_EQ(BlobFree(blob), RS_INVALID_PARAM);
}