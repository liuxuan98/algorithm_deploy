/**
 * @file crypto_utils.h
 * @brief Cryptographic utility functions for model encryption/decryption
 * @copyright .
 */

#ifndef RAYSHAPE_KERNEL_UTILS_CODEC_CRYPTO_UTILS_H_
#define RAYSHAPE_KERNEL_UTILS_CODEC_CRYPTO_UTILS_H_

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
             * @brief Encryption configuration structure
             */
            struct EncryptionConfig {
                std::string algorithm = "AES-256-GCM"; // Default encryption algorithm
                std::string key_derivation = "PBKDF2"; // Key derivation function
                int iterations = 100000;               // PBKDF2 iterations
                size_t key_length = 32;                // AES-256 key length (32 bytes)
                size_t iv_length = 12;                 // GCM IV length (12 bytes)
                size_t tag_length = 16;                // GCM authentication tag length (16 bytes)
                size_t salt_length = 16;               // Salt length for key derivation (16 bytes)

                EncryptionConfig() = default;
            };

            /**
             * @brief Encrypted data structure
             */
            struct EncryptedData {
                std::vector<uint8_t> ciphertext; // Encrypted data
                std::vector<uint8_t> iv;         // Initialization vector
                std::vector<uint8_t> tag;        // Authentication tag
                std::vector<uint8_t> salt;       // Salt for key derivation
                std::string algorithm;           // Algorithm used
                int iterations;                  // PBKDF2 iterations used

                EncryptedData() = default;
            };

            /**
             * @brief Cryptographic utility class for model encryption/decryption
             *
             * This class provides secure encryption and decryption functionality using
             * AES-256-GCM algorithm with PBKDF2 key derivation. It's designed to be
             * cross-platform and secure for protecting model data.
             */
            class RS_PUBLIC CryptoUtils {
            public:
                CryptoUtils() = default;
                ~CryptoUtils() = default;

                /**
                 * @brief Encrypt data using AES-256-GCM with password-based encryption
                 * @param plaintext Input data to encrypt
                 * @param password Password for encryption
                 * @param config Encryption configuration (optional)
                 * @param encrypted_data Output encrypted data structure
                 * @return true if successful, false otherwise
                 */
                static bool EncryptData(const std::vector<uint8_t> &plaintext,
                                        const std::string &password, const EncryptionConfig &config,
                                        EncryptedData &encrypted_data);

                /**
                 * @brief Encrypt string data
                 * @param plaintext Input string to encrypt
                 * @param password Password for encryption
                 * @param config Encryption configuration (optional)
                 * @param encrypted_data Output encrypted data structure
                 * @return true if successful, false otherwise
                 */
                static bool EncryptString(const std::string &plaintext, const std::string &password,
                                          const EncryptionConfig &config,
                                          EncryptedData &encrypted_data);

                /**
                 * @brief Decrypt data using AES-256-GCM
                 * @param encrypted_data Input encrypted data structure
                 * @param password Password for decryption
                 * @param plaintext Output decrypted data
                 * @return true if successful, false otherwise
                 */
                static bool DecryptData(const EncryptedData &encrypted_data,
                                        const std::string &password,
                                        std::vector<uint8_t> &plaintext);

                /**
                 * @brief Decrypt to string data
                 * @param encrypted_data Input encrypted data structure
                 * @param password Password for decryption
                 * @param plaintext Output decrypted string
                 * @return true if successful, false otherwise
                 */
                static bool DecryptString(const EncryptedData &encrypted_data,
                                          const std::string &password, std::string &plaintext);

                /**
                 * @brief Serialize encrypted data to binary format
                 * @param encrypted_data Input encrypted data
                 * @param output Output binary data
                 * @return true if successful, false otherwise
                 */
                static bool SerializeEncryptedData(const EncryptedData &encrypted_data,
                                                   std::vector<uint8_t> &output);

                /**
                 * @brief Deserialize encrypted data from binary format
                 * @param input Input binary data
                 * @param encrypted_data Output encrypted data structure
                 * @return true if successful, false otherwise
                 */
                static bool DeserializeEncryptedData(const std::vector<uint8_t> &input,
                                                     EncryptedData &encrypted_data);

                /**
                 * @brief Generate secure random bytes
                 * @param length Number of bytes to generate
                 * @param output Output random bytes
                 * @return true if successful, false otherwise
                 */
                static bool GenerateRandomBytes(size_t length, std::vector<uint8_t> &output);

                /**
                 * @brief Derive key from password using PBKDF2
                 * @param password Input password
                 * @param salt Salt for key derivation
                 * @param iterations Number of iterations
                 * @param key_length Desired key length
                 * @param derived_key Output derived key
                 * @return true if successful, false otherwise
                 */
                static bool DeriveKey(const std::string &password, const std::vector<uint8_t> &salt,
                                      int iterations, size_t key_length,
                                      std::vector<uint8_t> &derived_key);

                /**
                 * @brief Validate password strength
                 * @param password Password to validate
                 * @return true if password meets minimum requirements, false otherwise
                 */
                static bool ValidatePassword(const std::string &password);

                /**
                 * @brief Generate secure random password
                 * @param length Password length (minimum 12, recommended 16+)
                 * @param include_symbols Whether to include special symbols
                 * @return Generated password string
                 */
                static std::string GeneratePassword(size_t length = 16,
                                                    bool include_symbols = true);

                /**
                 * @brief Get last error message
                 * @return Error message string
                 */
                static const std::string &GetLastError();

                /**
                 * @brief Check if CryptoPP library is available and working
                 * @return true if available, false otherwise
                 */
                static bool IsCryptoAvailable();

                /**
                 * @brief Get supported encryption algorithms
                 * @return Vector of supported algorithm names
                 */
                static std::vector<std::string> GetSupportedAlgorithms();

            private:
                /**
                 * @brief Set last error message (thread-safe)
                 * @param error Error message
                 */
                static void SetError(const std::string &error);

                /**
                 * @brief Clear last error message
                 */
                static void ClearError();

                // Delete copy constructor and assignment operator
                CryptoUtils(const CryptoUtils &) = delete;
                CryptoUtils &operator=(const CryptoUtils &) = delete;
            };

            /**
             * @brief RAII wrapper for secure password handling
             */
            class SecureString {
            public:
                explicit SecureString(const std::string &data);
                explicit SecureString(const char *data);
                ~SecureString();

                const char *c_str() const {
                    return data_.c_str();
                }
                const std::string &str() const {
                    return data_;
                }
                size_t length() const {
                    return data_.length();
                }
                bool empty() const {
                    return data_.empty();
                }

                // Delete copy operations to prevent accidental copies
                SecureString(const SecureString &) = delete;
                SecureString &operator=(const SecureString &) = delete;

                // Allow move operations
                SecureString(SecureString &&) noexcept;
                SecureString &operator=(SecureString &&) noexcept;

            private:
                std::string data_;
                void SecureZero();
            };

        } // namespace codec
    }     // namespace utils
} // namespace rayshape

#endif // RAYSHAPE_KERNEL_UTILS_CODEC_CRYPTO_UTILS_H_
