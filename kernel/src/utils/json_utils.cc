#include "utils/json_utils.h"

namespace rayshape
{
    namespace utils
    {
        ErrorCode RSJsonCreate(void *content, unsigned int content_size,
                               RSJsonHandle *json_handle) {
            ErrorCode ret = RS_SUCCESS;
            if (content == nullptr || content_size == 0) {
                RS_LOGE("content is null or content_size is 0\n");
                return RS_INVALID_PARAM;
            }

            if (json_handle == nullptr) {
                RS_LOGE("Invalid json handle pointer.\n");
                return RS_INVALID_PARAM;
            }

            rapidjson::Document *tmp_json_handle = new rapidjson::Document();
            if (!tmp_json_handle->Parse((char *)content, content_size).HasParseError()) {
                RS_LOGI("json parse success\n");
                ret = RS_SUCCESS;
            } else {
                RS_LOGE("json parse failed\n");
                delete tmp_json_handle;
                tmp_json_handle = nullptr;
                ret = RS_INVALID_JSON;
            }

            *json_handle = tmp_json_handle;
            return ret;
        }

        ErrorCode RSJsonCreate(const std::string &content, RSJsonHandle *json_handle) {
            ErrorCode ret = RS_SUCCESS;
            if (json_handle == nullptr) {
                RS_LOGE("Invalid json handle pointer.\n");
                return RS_INVALID_PARAM;
            }

            rapidjson::Document *tmp_json_handle = new rapidjson::Document();
            if (!tmp_json_handle->Parse(content.c_str(), content.size()).HasParseError()) {
                RS_LOGI("json parse success\n");
                ret = RS_SUCCESS;
            } else {
                RS_LOGE("json parse failed\n");
                delete tmp_json_handle;
                tmp_json_handle = nullptr;
                ret = RS_INVALID_JSON;
            }

            *json_handle = tmp_json_handle;
            return ret;
        }

        void RSJsonDestory(RSJsonHandle *json_handle) {
            if (json_handle != nullptr && *json_handle != nullptr) {
                delete *json_handle;
                *json_handle = nullptr;
            }
        }

        RSJsonObject RSJsonRootGet(RSJsonHandle json_handle) {
            if (json_handle != nullptr && ((*json_handle).IsObject() || (*json_handle).IsArray())) {
                return &(*json_handle);
            } else {
                RS_LOGE("JSON handle is null or root is not an object or array.\n");
                return nullptr;
            }
        }

        RSJsonObject RSJsonObjectGet(RSJsonObject json_object, const char *key) {
            if ((*json_object).HasMember(key)) {
                return &(*json_object)[key];
            } else {
                RS_LOGE("key: %s's value is not exists.\n", key);
                return nullptr;
            }
        }

        const char *RSJsonStringGet(RSJsonObject str_object) {
            if (str_object != nullptr && str_object->IsString()) {
                return str_object->GetString();
            } else {
                RS_LOGE("Invalid JSON value for string get.\n");
                return "";
            }
        }

        int RSJsonIntGet(RSJsonObject int_object, int default_value) {
            if (int_object != nullptr && int_object->IsInt()) {
                return int_object->GetInt();
            } else {
                return default_value;
            }
        }

        bool RSJsonBoolGet(RSJsonObject bool_object, bool default_value) {
            if (bool_object != nullptr && bool_object->IsBool()) {
                return bool_object->GetBool();
            } else {
                return default_value;
            }
        }

        RSJsonObject RSJsonArrayAt(RSJsonObject arr_object, unsigned int index) {
            if (arr_object == nullptr || !arr_object->IsArray()) {
                RS_LOGE("Invalid JSON array object.\n");
                return nullptr;
            }

            if (index >= (unsigned int)arr_object->Size()) {
                RS_LOGE("Index out of json array bounds.\n");
                return nullptr;
            }

            rapidjson::Value &element = (*arr_object)[index];

            return &element;
        }

        unsigned int RSJsonArraySize(const RSJsonObject arr_object) {
            if (arr_object != nullptr && arr_object->IsArray()) {
                return arr_object->Size();
            } else {
                RS_LOGW("JSON object is null or not an array.\n");
                return 0;
            }
        }

    } // namespace utils
} // namespace rayshape
