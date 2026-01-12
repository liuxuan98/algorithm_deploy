#include "gtest/gtest.h"

int add2(int a, int b)
{
    return a + b;
}

// 编写测试case
TEST(testCase, test0)
{
    EXPECT_EQ(add2(2, 3), 5); // 判断结果是不是等于5，EXPECT_EQ表示 "等于"
}
