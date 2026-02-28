/**
 * @file auto_crypto.cpp
 * @brief Automatic encryption/decryption utility implementation
 * @copyright .
 */

#include "utils/codec/auto_crypto.h"

#include <iostream>
#include <sstream>
#include <mutex>
#include <random>
#include <iomanip>
#include <fstream>

// CryptoPP includes for hashing
#include <cryptopp/sha.h>
#include <cryptopp/hex.h>
#include <cryptopp/filters.h>

namespace rayshape
{
    namespace utils
    {
        namespace codec
        {
            // Define static constexpr members for C++14 compatibility
            constexpr const char *AutoCrypto::AUTO_MAGIC;
            constexpr uint32_t AutoCrypto::AUTO_VERSION;

            // Thread-safe error handling
            namespace
            {
                std::mutex error_mutex;
                std::string last_error;

                void SetThreadSafeError(const std::string &error) {
                    std::lock_guard<std::mutex> lock(error_mutex);
                    last_error = error;
                    std::cerr << "[AutoCrypto Error] " << error << std::endl;
                }

                std::string GetThreadSafeError() {
                    std::lock_guard<std::mutex> lock(error_mutex);
                    return last_error;
                }

                void ClearThreadSafeError() {
                    std::lock_guard<std::mutex> lock(error_mutex);
                    last_error.clear();
                }
            } // namespace

            bool AutoCrypto::AutoEncrypt(const std::vector<uint8_t> &plaintext,
                                         std::vector<uint8_t> &encrypted_output) {
                ClearError();

                if (plaintext.empty()) {
                    SetError("Plaintext is empty");
                    return false;
                }

                if (!IsAvailable()) {
                    SetError("Auto-encryption is not available");
                    return false;
                }

                try {
                    // Generate automatic key based on content and system
                    std::string auto_key;
                    if (!GenerateAutoKey(plaintext, auto_key)) {
                        return false;
                    }

                    // Encrypt using the auto-generated key
                    EncryptionConfig config;
                    EncryptedData encrypted_data;

                    if (!CryptoUtils::EncryptData(plaintext, auto_key, config, encrypted_data)) {
                        SetError("Failed to encrypt data: " + CryptoUtils::GetLastError());
                        return false;
                    }

                    // Create auto-encryption wrapper format
                    std::ostringstream oss(std::ios::binary);

                    // Write auto-encryption header
                    oss.write(AUTO_MAGIC, 4);
                    oss.write(reinterpret_cast<const char *>(&AUTO_VERSION), sizeof(AUTO_VERSION));

                    // Store content hash for key regeneration during decryption
                    std::string content_hash = GenerateContentHash(plaintext);
                    uint32_t hash_size = static_cast<uint32_t>(content_hash.size());
                    oss.write(reinterpret_cast<const char *>(&hash_size), sizeof(hash_size));
                    oss.write(content_hash.c_str(), content_hash.size());

                    // Serialize the encrypted data
                    std::vector<uint8_t> serialized_encrypted;
                    if (!CryptoUtils::SerializeEncryptedData(encrypted_data,
                                                             serialized_encrypted)) {
                        SetError("Failed to serialize encrypted data: "
                                 + CryptoUtils::GetLastError());
                        return false;
                    }

                    // Write encrypted data size and content
                    uint32_t data_size = static_cast<uint32_t>(serialized_encrypted.size());
                    oss.write(reinterpret_cast<const char *>(&data_size), sizeof(data_size));
                    oss.write(reinterpret_cast<const char *>(serialized_encrypted.data()),
                              serialized_encrypted.size());

                    std::string output_data = oss.str();
                    encrypted_output.assign(output_data.begin(), output_data.end());

                    return true;

                } catch (const std::exception &e) {
                    SetError("Exception during auto-encryption: " + std::string(e.what()));
                    return false;
                }
            }

            bool AutoCrypto::AutoEncrypt(const std::string &plaintext,
                                         std::vector<uint8_t> &encrypted_output) {
                std::vector<uint8_t> data(plaintext.begin(), plaintext.end());
                return AutoEncrypt(data, encrypted_output);
            }

            bool AutoCrypto::AutoDecrypt(const std::vector<uint8_t> &encrypted_data,
                                         std::vector<uint8_t> &plaintext) {
                ClearError();

                if (encrypted_data.empty()) {
                    SetError("Encrypted data is empty");
                    return false;
                }

                if (!IsAutoEncrypted(encrypted_data)) {
                    SetError("Data is not auto-encrypted");
                    return false;
                }

                if (!IsAvailable()) {
                    SetError("Auto-decryption is not available");
                    return false;
                }

                try {
                    std::istringstream iss(
                        std::string(encrypted_data.begin(), encrypted_data.end()),
                        std::ios::binary);

                    // Skip header (already verified)
                    iss.seekg(4, std::ios::cur); // magic
                    uint32_t version;
                    iss.read(reinterpret_cast<char *>(&version), sizeof(version));

                    // Read stored content hash
                    uint32_t hash_size;
                    iss.read(reinterpret_cast<char *>(&hash_size), sizeof(hash_size));

                    std::string stored_content_hash(hash_size, '\0');
                    iss.read(&stored_content_hash[0], hash_size);

                    // Read encrypted data
                    uint32_t data_size;
                    iss.read(reinterpret_cast<char *>(&data_size), sizeof(data_size));

                    std::vector<uint8_t> serialized_encrypted(data_size);
                    iss.read(reinterpret_cast<char *>(serialized_encrypted.data()), data_size);

                    // Deserialize encrypted data
                    EncryptedData encrypted_info;
                    if (!CryptoUtils::DeserializeEncryptedData(serialized_encrypted,
                                                               encrypted_info)) {
                        SetError("Failed to deserialize encrypted data: "
                                 + CryptoUtils::GetLastError());
                        return false;
                    }

                    // Use the stored content hash as the decryption key
                    // This allows cross-platform decryption of the same file content
                    if (!CryptoUtils::DecryptData(encrypted_info, stored_content_hash, plaintext)) {
                        SetError("Failed to decrypt data: " + CryptoUtils::GetLastError());
                        return false;
                    }

                    return true;

                } catch (const std::exception &e) {
                    SetError("Exception during auto-decryption: " + std::string(e.what()));
                    return false;
                }
            }

            bool AutoCrypto::AutoDecrypt(const std::vector<uint8_t> &encrypted_data,
                                         std::string &plaintext) {
                std::vector<uint8_t> data;
                if (!AutoDecrypt(encrypted_data, data)) {
                    return false;
                }
                plaintext.assign(data.begin(), data.end());
                return true;
            }

            bool AutoCrypto::IsAutoEncrypted(const std::vector<uint8_t> &data) {
                if (data.size() < 12) { // Minimum size: magic + version + hash_size
                    return false;
                }

                std::string magic(data.begin(), data.begin() + 4);
                return magic == AUTO_MAGIC;
            }

            bool AutoCrypto::IsFileAutoEncrypted(const std::string &filename) {
                std::ifstream ifs(filename, std::ios::binary);
                if (!ifs.is_open()) {
                    return false;
                }

                std::vector<uint8_t> header(12);
                ifs.read(reinterpret_cast<char *>(header.data()), header.size());
                if (!ifs.good()) {
                    return false;
                }

                return IsAutoEncrypted(header);
            }

            bool AutoCrypto::GenerateAutoKey(const std::vector<uint8_t> &context_data,
                                             std::string &key) {
                try {
                    // Create a deterministic key based on file content hash
                    // This allows cross-platform decryption of the same file
                    std::string content_hash = GenerateContentHash(context_data);

                    // Use content hash as the primary key source for portability
                    key = content_hash;

                    // Validate the key meets minimum requirements
                    if (!CryptoUtils::ValidatePassword(key)) {
                        // If content hash is not strong enough, enhance it
                        key = content_hash + "_RayShapeAuto_" + std::to_string(context_data.size());
                    }

                    return true;

                } catch (const std::exception &e) {
                    SetError("Failed to generate auto key: " + std::string(e.what()));
                    return false;
                }
            }

            std::string AutoCrypto::GenerateContentHash(const std::vector<uint8_t> &content) {
                try {
                    // Generate SHA256 hash of the content for deterministic key generation
                    std::string hash_result;

                    CryptoPP::SHA256 sha256;
                    CryptoPP::StringSource(content.data(), content.size(), true,
                                           new CryptoPP::HashFilter(
                                               sha256, new CryptoPP::HexEncoder(
                                                           new CryptoPP::StringSink(hash_result))));

                    // Use first 16 characters + fixed suffix for a stable key
                    return hash_result.substr(0, 16) + "RayShapeContent";

                } catch (const std::exception &e) {
                    // Fallback to a content-size based key if hashing fails
                    return "RayShapeContentKey" + std::to_string(content.size()) + "2025";
                }
            }

            const std::string &AutoCrypto::GetLastError() {
                static std::string error = GetThreadSafeError();
                error = GetThreadSafeError(); // Update the static string
                return error;
            }

            bool AutoCrypto::IsAvailable() {
                return CryptoUtils::IsCryptoAvailable();
            }

            void AutoCrypto::SetError(const std::string &error) {
                SetThreadSafeError(error);
            }

            void AutoCrypto::ClearError() {
                ClearThreadSafeError();
            }

        } // namespace codec
    }     // namespace utils
} // namespace rayshape
