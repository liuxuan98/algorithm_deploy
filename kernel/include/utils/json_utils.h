#ifndef _JSON_UTILS_H_
#define _JSON_UTILS_H_

#include "base/error.h"
#include "rapidjson_include.h"

namespace rayshape
{
    namespace utils
    {

        typedef rapidjson::Document *RSJsonHandle;

        typedef rapidjson::Value *RSJsonObject;

        ErrorCode RSJsonCreate(void *content, unsigned int content_size, RSJsonHandle *json_handle);

        ErrorCode RSJsonCreate(const std::string &content,
                               RSJsonHandle *json_handle); // 重载一个创建按函数用于测试

        void RSJsonDestory(RSJsonHandle *json_handle);

        RSJsonObject RSJsonRootGet(RSJsonHandle json_handle);

        RSJsonObject RSJsonObjectGet(RSJsonObject json_object, const char *key);

        const char *RSJsonStringGet(RSJsonObject str_object);

        int RSJsonIntGet(RSJsonObject int_object, int default_value);

        RSJsonObject RSJsonArrayAt(RSJsonObject arr_object, unsigned int index);

        unsigned int RSJsonArraySize(const RSJsonObject arr_object);

    } // namespace utils
} // namespace rayshape

#endif
