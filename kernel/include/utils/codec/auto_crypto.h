/**
 * @file auto_crypto.h
 * @brief Automatic encryption/decryption utility with transparent key management
 * @copyright .
 */

#ifndef RAYSHAPE_KERNEL_UTILS_CODEC_AUTO_CRYPTO_H_
#define RAYSHAPE_KERNEL_UTILS_CODEC_AUTO_CRYPTO_H_

#include "crypto_utils.h"
#include "base/macros.h"
#include <string>
#include <vector>
#include <memory>

namespace rayshape
{
    namespace utils
    {
        namespace codec
        {

            /**
             * @brief Automatic encryption manager with transparent key handling
             *
             * This class provides transparent encryption/decryption without requiring
             * users to manage passwords or keys manually. It automatically generates
             * secure keys based on content for cross-platform compatibility.
             */
            class RS_PUBLIC AutoCrypto {
            public:
                /**
                 * @brief Automatically encrypt data
                 * @param plaintext Input data to encrypt
                 * @param encrypted_output Output encrypted data
                 * @return true if successful, false otherwise
                 */
                static bool AutoEncrypt(const std::vector<uint8_t> &plaintext,
                                        std::vector<uint8_t> &encrypted_output);

                /**
                 * @brief Automatically encrypt string data
                 * @param plaintext Input string to encrypt
                 * @param encrypted_output Output encrypted data
                 * @return true if successful, false otherwise
                 */
                static bool AutoEncrypt(const std::string &plaintext,
                                        std::vector<uint8_t> &encrypted_output);

                /**
                 * @brief Automatically decrypt data
                 * @param encrypted_data Input encrypted data
                 * @param plaintext Output decrypted data
                 * @return true if successful, false otherwise
                 */
                static bool AutoDecrypt(const std::vector<uint8_t> &encrypted_data,
                                        std::vector<uint8_t> &plaintext);

                /**
                 * @brief Automatically decrypt to string
                 * @param encrypted_data Input encrypted data
                 * @param plaintext Output decrypted string
                 * @return true if successful, false otherwise
                 */
                static bool AutoDecrypt(const std::vector<uint8_t> &encrypted_data,
                                        std::string &plaintext);

                /**
                 * @brief Check if data is auto-encrypted
                 * @param data Input data to check
                 * @return true if data appears to be auto-encrypted, false otherwise
                 */
                static bool IsAutoEncrypted(const std::vector<uint8_t> &data);

                /**
                 * @brief Check if file contains auto-encrypted data
                 * @param filename Path to file
                 * @return true if file appears to be auto-encrypted, false otherwise
                 */
                static bool IsFileAutoEncrypted(const std::string &filename);

                /**
                 * @brief Get last error message
                 * @return Error message string
                 */
                static const std::string &GetLastError();

                /**
                 * @brief Check if auto-encryption is available
                 * @return true if available, false otherwise
                 */
                static bool IsAvailable();

            private:
                /**
                 * @brief Generate automatic encryption key
                 * @param context_data Context data for key generation
                 * @param key Output encryption key
                 * @return true if successful, false otherwise
                 */
                static bool GenerateAutoKey(const std::vector<uint8_t> &context_data,
                                            std::string &key);

                /**
                 * @brief Generate content-based hash for key generation
                 * @param content Content data to hash
                 * @return Content hash string
                 */
                static std::string GenerateContentHash(const std::vector<uint8_t> &content);

                /**
                 * @brief Set last error message
                 * @param error Error message
                 */
                static void SetError(const std::string &error);

                /**
                 * @brief Clear last error message
                 */
                static void ClearError();

                // Magic bytes for auto-encrypted data
                static constexpr const char *AUTO_MAGIC = "RAEC"; // RayShape Auto Encrypted
                static constexpr uint32_t AUTO_VERSION = 1;
            };

        } // namespace codec
    }     // namespace utils
} // namespace rayshape

#endif // RAYSHAPE_KERNEL_UTILS_CODEC_AUTO_CRYPTO_H_
