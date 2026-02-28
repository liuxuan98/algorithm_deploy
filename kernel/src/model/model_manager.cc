/**
 * @file model_manager.cc
 * @brief Model loading and management utility implementation
 * @copyright
 */

#include "model/model_manager.h"
#include "utils/codec/model_parse.h"
#include "base/logger.h"

#include <fstream>
#include <sys/stat.h>
#include <cstdio>

#ifdef _WIN32
#include <io.h>
#ifndef S_ISREG
#define S_ISREG(m) (((m)&S_IFMT) == S_IFREG)
#endif
#endif

namespace rayshape
{

    ModelManager &ModelManager::Instance() {
        static ModelManager instance;
        return instance;
    }

    std::unique_ptr<Model> ModelManager::LoadFromFile(const std::string &model_path) {
        // Validate file first
        ErrorCode ret = ValidateModelFile(model_path);
        if (ret != RS_SUCCESS) {
            RS_LOGE("Model file validation failed: %s, error: %d\n", model_path.c_str(), ret);
            return nullptr;
        }

        try {
            // Load using ParseModel function for cereal serialized models
            auto model = ParseModel(model_path);
            if (model != nullptr) {
                RS_LOGD("Successfully loaded serialized model from: %s\n", model_path.c_str());
                return model;
            }
        } catch (const std::exception &e) {
            RS_LOGE("Failed to parse serialized model: %s, error: %s\n", model_path.c_str(),
                    e.what());
        }

        return nullptr;
    }

    std::unique_ptr<Model> ModelManager::LoadFromMemory(const char *model_buf, std::size_t size) {
        if (model_buf == nullptr || size == 0) {
            RS_LOGE("Invalid model buffer parameters\n");
            return nullptr;
        }

        try {
            return ParseModel(model_buf, size);
        } catch (const std::exception &e) {
            RS_LOGE("Failed to parse model from memory: %s\n", e.what());
            return nullptr;
        }
    }

    ErrorCode ModelManager::ValidateModelFile(const std::string &model_path) {
        if (model_path.empty()) {
            return RS_INVALID_PARAM;
        }

        try {
            // Check if file exists using stat
            struct stat file_stat;
            if (stat(model_path.c_str(), &file_stat) != 0) {
                RS_LOGE("Model file does not exist: %s\n", model_path.c_str());
                return RS_FILE_OPEN_ERROR;
            }

            // Check if it's a regular file
            if (!S_ISREG(file_stat.st_mode)) {
                RS_LOGE("Path is not a regular file: %s\n", model_path.c_str());
                return RS_INVALID_FILE;
            }

            // Check if file is readable
            std::ifstream file(model_path, std::ios::binary);
            if (!file.is_open()) {
                RS_LOGE("Cannot open model file for reading: %s\n", model_path.c_str());
                return RS_FILE_OPEN_ERROR;
            }
            file.close();

        } catch (const std::exception &e) {
            RS_LOGE("Exception while validating model file: %s\n", e.what());
            return RS_FILE_OPEN_ERROR;
        }

        return RS_SUCCESS;
    }

    // Convenience functions implementation
    std::unique_ptr<Model> LoadModel(const std::string &model_path) {
        return ModelManager::Instance().LoadFromFile(model_path);
    }

} // namespace rayshape