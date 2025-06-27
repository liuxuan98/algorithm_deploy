#include "gtest/gtest.h"
#include "inference/inference.h"

using namespace rayshape;
using namespace rayshape::inference;
// 测试 CreateInference 是否能成功创建指定类型的实例
TEST(InferenceFactoryTest, CreateValidType) {
    InferenceType type = INFERENCE_TYPE_OPENVINO;
    auto inference = CreateInference(type);

    EXPECT_NE(inference, nullptr) << "CreateInference should return a valid object for known type.";
}

// 测试不支持的类型是否会返回 nullptr
TEST(InferenceFactoryTest, CreateInvalidType) {
    InferenceType type = static_cast<InferenceType>(-1); // 假设是无效类型
    auto inference = CreateInference(type);

    EXPECT_EQ(inference, nullptr) << "CreateInference should return nullptr for unsupported types.";
}

// 使用参数化测试多个类型（可选）
// struct InferenceTypeCase
//{
//    InferenceType type;
//    bool shouldSucceed;
//};
//
// class InferenceFactoryParamTest : public ::testing::TestWithParam<InferenceTypeCase>
//{
//};
//
// INSTANTIATE_TEST_SUITE_P(
//    InferenceType,
//    InferenceFactoryParamTest,
//    ::testing::Values(
//        InferenceTypeCase{INFERENCE_TYPE_OPENVINO, true},
//        InferenceTypeCase{static_cast<InferenceType>(0), false}));
//
// TEST_P(InferenceFactoryParamTest, CreateWithType)
//{
//    const auto &param = GetParam();
//    auto inference = CreateInference(param.type);
//
//    if (param.shouldSucceed)
//    {
//        EXPECT_NE(inference, nullptr) << "Expected non-null for type: " << param.type;
//    }
//    else
//    {
//        EXPECT_EQ(inference, nullptr) << "Expected null for invalid type: " << param.type;
//    }
//}
