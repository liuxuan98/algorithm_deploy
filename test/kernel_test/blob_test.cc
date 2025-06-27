#include "memory_manager/blob.h"
#include "gtest/gtest.h"

using namespace rayshape;
TEST(BlobTest, BlobAllocTest) {
    Dims dims{ 4, { 1, 3, 224, 224 } };
    Blob *blob_0 = BlobAlloc(DEVICE_TYPE_X86, DATA_TYPE_FLOAT, DATA_FORMAT_NCHW, "x86_blob", &dims);
    EXPECT_TRUE(blob_0);
    BlobFree(blob_0);
    blob_0 = nullptr; // set null by manual.

    Blob *blob_1 = BlobAlloc(DEVICE_TYPE_X86, DATA_TYPE_FLOAT, DATA_FORMAT_NCHW, nullptr, &dims);
    EXPECT_FALSE(blob_1);

    Blob *blob_2 =
        BlobAlloc(DEVICE_TYPE_X86, DATA_TYPE_FLOAT, DATA_FORMAT_NCHW, "cpu_blob", nullptr);
    EXPECT_FALSE(blob_2);
}

TEST(BlobTest, CopyBetweenBlobs_ShouldSuccess) {
    Dims dims{ 4, { 1, 3, 224, 224 } };
    Blob *src = BlobAlloc(DEVICE_TYPE_X86, DATA_TYPE_FLOAT, DATA_FORMAT_NCHW, "cpu_blob", &dims);
    Blob *dst =
        BlobAlloc(DEVICE_TYPE_X86, DATA_TYPE_FLOAT, DATA_FORMAT_NCHW, "another_cpu_blob", &dims);

    // 填充一些数据
    float *src_data = static_cast<float *>(src->buffer->GetDataPtr());
    for (int i = 0; i < 3 * 224 * 224; ++i) {
        src_data[i] = static_cast<float>(i);
    }

    EXPECT_EQ(BlobCopy(dst, src), RS_SUCCESS);

    float *dst_data = static_cast<float *>(dst->buffer->GetDataPtr());
    for (int i = 0; i < 3 * 224 * 224; ++i) {
        EXPECT_EQ(dst_data[i], static_cast<float>(i));
    }

    BlobFree(src);
    BlobFree(dst);
}

TEST(BlobTest, FreeBlob_ShouldSuccess) {
    Dims dims{ 4, { 1, 3, 224, 224 } };
    Blob *blob = BlobAlloc(DEVICE_TYPE_X86, DATA_TYPE_FLOAT, DATA_FORMAT_NCHW, "test_blob", &dims);

    EXPECT_NE(blob, nullptr);

    EXPECT_EQ(BlobFree(blob), RS_SUCCESS);

    // Blob *my_blob = BlobAlloc(...);
    // BlobFree(my_blob);
    // my_blob = nullptr; // 防止野指针访问
}