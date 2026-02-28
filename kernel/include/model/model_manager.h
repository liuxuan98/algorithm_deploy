/**
 * @file model_manager.h
 * @brief Model loading and management utility
 * @copyright .
 */

#ifndef MODEL_MANAGER_H
#define MODEL_MANAGER_H

#include "base/common.h"
#include "base/error.h"
#include "model/model.h"

#include <memory>
#include <string>

namespace rayshape
{

    /**
     * @brief Model loading and management utility class
     * @details Provides unified interface for loading models from various sources
     *          and converting them to appropriate Model objects for inference engines
     */
    class RS_PUBLIC ModelManager {
    public:
        /**
         * @brief Default constructor
         */
        ModelManager() = default;

        /**
         * @brief Destructor
         */
        ~ModelManager() = default;

        // Non-copyable and non-movable for singleton-like behavior
        ModelManager(const ModelManager &) = delete;
        ModelManager &operator=(const ModelManager &) = delete;
        ModelManager(ModelManager &&) = delete;
        ModelManager &operator=(ModelManager &&) = delete;

        /**
         * @brief Get singleton instance
         * @return ModelManager& Reference to singleton instance
         */
        static ModelManager &Instance();

        /**
         * @brief Load model from serialized file
         * @param[in] model_path Path to the serialized model file
         * @return std::unique_ptr<Model> Loaded model object, nullptr if failed
         */
        std::unique_ptr<Model> LoadFromFile(const std::string &model_path);

        /**
         * @brief Load model from memory buffer
         * @param[in] model_buf Pointer to model data buffer
         * @param[in] size Size of the model data buffer
         * @return std::unique_ptr<Model> Loaded model object, nullptr if failed
         */
        std::unique_ptr<Model> LoadFromMemory(const char *model_buf, std::size_t size);

        /**
         * @brief Validate model file exists and is readable
         * @param[in] model_path Path to model file
         * @return ErrorCode RS_SUCCESS if valid, error code otherwise
         */
        ErrorCode ValidateModelFile(const std::string &model_path);
    };

    /**
     * @brief Convenience function to load model from file
     * @param[in] model_path Path to model file
     * @return std::unique_ptr<Model> Loaded model, nullptr if failed
     */
    RS_PUBLIC std::unique_ptr<Model> LoadModel(const std::string &model_path);

} // namespace rayshape

#endif // MODEL_MANAGER_H
