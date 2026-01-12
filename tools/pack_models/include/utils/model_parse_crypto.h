/**
 * @file model_parse_crypto.h
 * @brief Model parsing with encryption support for pack_models tool
 * @copyright (c) 2025 Shenzhen RayShape Medical Technology Co., Ltd.
 */

#ifndef PACK_MODELS_UTILS_MODEL_PARSE_CRYPTO_H_
#define PACK_MODELS_UTILS_MODEL_PARSE_CRYPTO_H_

#include "model/model.h"
#include "utils/codec/model_codec.h"
#include <memory>
#include <string>

namespace rayshape
{
    namespace tools
    {
        namespace utils
        {

            /**
             * @brief Model parser with encryption support
             */
            class ModelParserCrypto {
            public:
                /**
                 * @brief Parse model from file (with optional decryption)
                 * @param filename Path to serialized model file
                 * @param password Password for decryption (empty = no decryption)
                 * @return std::unique_ptr<Model> Parsed model object
                 */
                static std::unique_ptr<rayshape::Model> ParseModel(const std::string &filename,
                                                                  const std::string &password = "");

                /**
                 * @brief Parse model from memory buffer (with optional decryption)
                 * @param model_buf Pointer to model data buffer
                 * @param size Size of the model data buffer
                 * @param password Password for decryption (empty = no decryption)
                 * @return std::unique_ptr<Model> Parsed model object
                 */
                static std::unique_ptr<rayshape::Model> ParseModel(const char *model_buf, std::size_t size,
                                                                  const std::string &password = "");

                /**
                 * @brief Parse model from binary data vector (with optional decryption)
                 * @param model_data Binary model data
                 * @param password Password for decryption (empty = no decryption)
                 * @return std::unique_ptr<Model> Parsed model object
                 */
                static std::unique_ptr<rayshape::Model> ParseModel(const std::vector<uint8_t> &model_data,
                                                                  const std::string &password = "");

                /**
                 * @brief Get last error message
                 * @return Error message string
                 */
                static const std::string& GetLastError();

                /**
                 * @brief Check if data appears to be encrypted
                 * @param data Binary data to check
                 * @param size Size of data
                 * @return true if data appears encrypted, false otherwise
                 */
                static bool IsDataEncrypted(const char* data, std::size_t size);

                /**
                 * @brief Check if file appears to contain encrypted data
                 * @param filename Path to file
                 * @return true if file appears encrypted, false otherwise
                 */
                static bool IsFileEncrypted(const std::string &filename);

            private:
                /**
                 * @brief Parse decrypted model data using cereal
                 * @param decrypted_data Decrypted model data
                 * @return Parsed model object
                 */
                static std::unique_ptr<rayshape::Model> ParseDecryptedData(const std::vector<uint8_t> &decrypted_data);

                /**
                 * @brief Set last error message (thread-safe)
                 * @param error Error message
                 */
                static void SetError(const std::string &error);

                /**
                 * @brief Clear last error message
                 */
                static void ClearError();
            };

        } // namespace utils
    } // namespace tools
} // namespace rayshape

#endif // PACK_MODELS_UTILS_MODEL_PARSE_CRYPTO_H_
