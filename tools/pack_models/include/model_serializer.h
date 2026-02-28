/**
 * @file model_serializer.h
 * @brief Model serialization utility for packing models
 * @copyright (c) .
 */

#ifndef PACK_MODELS_MODEL_SERIALIZER_H_
#define PACK_MODELS_MODEL_SERIALIZER_H_

#include <string>
#include <memory>
#include <vector>

#include "base/common.h"
#include "model/model.h"
#include "utils/codec/model_codec.h"

namespace rayshape
{
    namespace tools
    {

        /**
         * @brief Model serialization utility class
         *
         * This class provides functionality to serialize different types of models
         * into a unified binary format using cereal serialization library.
         */
        class ModelSerializer {
        public:
            ModelSerializer() = default;
            ~ModelSerializer() = default;

            /**
             * @brief Serialize a model to binary file
             * @param model The model to serialize
             * @param output_path Output file path
             * @param auto_encrypt Enable automatic encryption
             * @return true if successful, false otherwise
             */
            bool SerializeModelToFile(std::unique_ptr<Model> model, const std::string &output_path,
                                     bool auto_encrypt = false);

            /**
             * @brief Serialize a model to binary buffer
             * @param model The model to serialize
             * @param buffer Output buffer
             * @param auto_encrypt Enable automatic encryption
             * @return true if successful, false otherwise
             */
            bool SerializeModelToBuffer(std::unique_ptr<Model> model, std::vector<uint8_t> &buffer,
                                       bool auto_encrypt = false);

            /**
             * @brief Create MNN model from file and serialize
             * @param mnn_model_path Path to MNN model file
             * @param config_path Path to configuration JSON file (optional)
             * @param output_path Output serialized file path
             * @param auto_encrypt Enable automatic encryption
             * @return true if successful, false otherwise
             */
            bool SerializeMnnModel(const std::string &mnn_model_path,
                                   const std::string &config_path, const std::string &output_path,
                                   bool auto_encrypt = false);

            /**
             * @brief Create OpenVINO model from files and serialize
             * @param xml_path Path to OpenVINO XML file
             * @param bin_path Path to OpenVINO BIN file
             * @param config_path Path to configuration JSON file (optional)
             * @param output_path Output serialized file path
             * @param auto_encrypt Enable automatic encryption
             * @return true if successful, false otherwise
             */
            bool SerializeOpenVINOModel(const std::string &xml_path, const std::string &bin_path,
                                        const std::string &config_path,
                                        const std::string &output_path,
                                        bool auto_encrypt = false);

            /**
             * @brief Create ONNX model from file and serialize
             * @param onnx_model_path Path to ONNX model file
             * @param config_path Path to configuration JSON file (optional)
             * @param output_path Output serialized file path
             * @param auto_encrypt Enable automatic encryption
             * @return true if successful, false otherwise
             */
            bool SerializeONNXModel(const std::string &onnx_model_path,
                                    const std::string &config_path, const std::string &output_path,
                                    bool auto_encrypt = false);

            /**
             * @brief Get last error message
             * @return Error message string
             */
            const std::string &GetLastError() const {
                return last_error_;
            }

            /**
             * @brief Validate model file before serialization
             * @param model_path Path to model file
             * @param model_type Expected model type
             * @return true if valid, false otherwise
             */
            bool ValidateModelFile(const std::string &model_path, rayshape::ModelType model_type);

        private:
            /**
             * @brief Load file content into string
             * @param file_path Path to file
             * @param content Output content
             * @return true if successful, false otherwise
             */
            bool LoadFileContent(const std::string &file_path, std::string &content);

            /**
             * @brief Set last error message
             * @param error Error message
             */
            void SetError(const std::string &error);

        private:
            std::string last_error_;
        };

    } // namespace tools
} // namespace rayshape

#endif // PACK_MODELS_MODEL_SERIALIZER_H_
