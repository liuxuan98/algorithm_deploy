/**
 * @file crypto_utils.cpp
 * @brief Cryptographic utility functions implementation
 * @copyright .
 */

#include "utils/codec/crypto_utils.h"

#include <iostream>
#include <sstream>
#include <random>
#include <algorithm>
#include <cstring>
#include <thread>
#include <mutex>

// CryptoPP includes
#include <cryptopp/aes.h>
#include <cryptopp/gcm.h>
#include <cryptopp/pwdbased.h>
#include <cryptopp/sha.h>
#include <cryptopp/osrng.h>
#include <cryptopp/hex.h>
#include <cryptopp/secblock.h>
#include <cryptopp/filters.h>

namespace rayshape
{
    namespace utils
    {
        namespace codec
        {

            // Thread-safe error handling
            namespace
            {
                std::mutex error_mutex;
                std::string last_error;

                void SetThreadSafeError(const std::string &error) {
                    std::lock_guard<std::mutex> lock(error_mutex);
                    last_error = error;
                    std::cerr << "[CryptoUtils Error] " << error << std::endl;
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

            bool CryptoUtils::EncryptData(const std::vector<uint8_t> &plaintext,
                                          const std::string &password,
                                          const EncryptionConfig &config,
                                          EncryptedData &encrypted_data) {
                try {
                    ClearError();

                    if (plaintext.empty()) {
                        SetError("Plaintext is empty");
                        return false;
                    }

                    if (!ValidatePassword(password)) {
                        SetError("Password does not meet minimum security requirements");
                        return false;
                    }

                    // Generate random salt
                    if (!GenerateRandomBytes(config.salt_length, encrypted_data.salt)) {
                        SetError("Failed to generate salt");
                        return false;
                    }

                    // Derive key from password
                    std::vector<uint8_t> derived_key;
                    if (!DeriveKey(password, encrypted_data.salt, config.iterations,
                                   config.key_length, derived_key)) {
                        SetError("Failed to derive encryption key");
                        return false;
                    }

                    // Generate random IV
                    if (!GenerateRandomBytes(config.iv_length, encrypted_data.iv)) {
                        SetError("Failed to generate IV");
                        return false;
                    }

                    // Prepare for encryption
                    CryptoPP::GCM<CryptoPP::AES>::Encryption encryptor;
                    encryptor.SetKeyWithIV(derived_key.data(), derived_key.size(),
                                           encrypted_data.iv.data(), encrypted_data.iv.size());

                    // Encrypt data
                    encrypted_data.ciphertext.resize(plaintext.size());
                    encrypted_data.tag.resize(config.tag_length);

                    // Use string sink to capture both ciphertext and tag
                    std::string ciphertext_and_tag;
                    CryptoPP::AuthenticatedEncryptionFilter ef(
                        encryptor, new CryptoPP::StringSink(ciphertext_and_tag));

                    ef.Put(plaintext.data(), plaintext.size());
                    ef.MessageEnd();

                    // Split ciphertext and tag
                    if (ciphertext_and_tag.size() < config.tag_length) {
                        SetError("Encrypted output too small to contain tag");
                        return false;
                    }

                    size_t ciphertext_size = ciphertext_and_tag.size() - config.tag_length;
                    encrypted_data.ciphertext.resize(ciphertext_size);
                    encrypted_data.tag.resize(config.tag_length);

                    std::memcpy(encrypted_data.ciphertext.data(), ciphertext_and_tag.data(),
                                ciphertext_size);
                    std::memcpy(encrypted_data.tag.data(),
                                ciphertext_and_tag.data() + ciphertext_size, config.tag_length);

                    // Set metadata
                    encrypted_data.algorithm = config.algorithm;
                    encrypted_data.iterations = config.iterations;

                    // Securely clear derived key
                    std::fill(derived_key.begin(), derived_key.end(), 0);

                    return true;

                } catch (const CryptoPP::Exception &e) {
                    SetError("CryptoPP exception: " + std::string(e.what()));
                    return false;
                } catch (const std::exception &e) {
                    SetError("Standard exception: " + std::string(e.what()));
                    return false;
                }
            }

            bool CryptoUtils::EncryptString(const std::string &plaintext,
                                            const std::string &password,
                                            const EncryptionConfig &config,
                                            EncryptedData &encrypted_data) {
                std::vector<uint8_t> data(plaintext.begin(), plaintext.end());
                return EncryptData(data, password, config, encrypted_data);
            }

            bool CryptoUtils::DecryptData(const EncryptedData &encrypted_data,
                                          const std::string &password,
                                          std::vector<uint8_t> &plaintext) {
                try {
                    ClearError();

                    if (encrypted_data.ciphertext.empty()) {
                        SetError("Ciphertext is empty");
                        return false;
                    }

                    if (encrypted_data.salt.empty() || encrypted_data.iv.empty()
                        || encrypted_data.tag.empty()) {
                        SetError("Invalid encrypted data structure");
                        return false;
                    }

                    // Derive key from password
                    std::vector<uint8_t> derived_key;
                    if (!DeriveKey(password, encrypted_data.salt, encrypted_data.iterations, 32,
                                   derived_key)) { // AES-256 key length
                        SetError("Failed to derive decryption key");
                        return false;
                    }

                    // Prepare for decryption
                    CryptoPP::GCM<CryptoPP::AES>::Decryption decryptor;
                    decryptor.SetKeyWithIV(derived_key.data(), derived_key.size(),
                                           encrypted_data.iv.data(), encrypted_data.iv.size());

                    // Decrypt data
                    plaintext.resize(encrypted_data.ciphertext.size());

                    CryptoPP::AuthenticatedDecryptionFilter df(
                        decryptor, new CryptoPP::ArraySink(plaintext.data(), plaintext.size()),
                        CryptoPP::AuthenticatedDecryptionFilter::MAC_AT_END,
                        encrypted_data.tag.size());

                    df.Put(encrypted_data.ciphertext.data(), encrypted_data.ciphertext.size());
                    df.Put(encrypted_data.tag.data(), encrypted_data.tag.size());
                    df.MessageEnd();

                    // Securely clear derived key
                    std::fill(derived_key.begin(), derived_key.end(), 0);

                    return true;

                } catch (const CryptoPP::HashVerificationFilter::HashVerificationFailed &) {
                    SetError("Authentication failed - incorrect password or corrupted data");
                    return false;
                } catch (const CryptoPP::Exception &e) {
                    SetError("CryptoPP exception: " + std::string(e.what()));
                    return false;
                } catch (const std::exception &e) {
                    SetError("Standard exception: " + std::string(e.what()));
                    return false;
                }
            }

            bool CryptoUtils::DecryptString(const EncryptedData &encrypted_data,
                                            const std::string &password, std::string &plaintext) {
                std::vector<uint8_t> data;
                if (!DecryptData(encrypted_data, password, data)) {
                    return false;
                }
                plaintext.assign(data.begin(), data.end());
                return true;
            }

            bool CryptoUtils::SerializeEncryptedData(const EncryptedData &encrypted_data,
                                                     std::vector<uint8_t> &output) {
                try {
                    ClearError();

                    // Binary format:
                    // [4 bytes] Magic: "RENC" (RayShape Encrypted)
                    // [4 bytes] Version
                    // [4 bytes] Algorithm name length
                    // [variable] Algorithm name
                    // [4 bytes] Iterations
                    // [4 bytes] Salt length
                    // [variable] Salt
                    // [4 bytes] IV length
                    // [variable] IV
                    // [4 bytes] Tag length
                    // [variable] Tag
                    // [4 bytes] Ciphertext length
                    // [variable] Ciphertext

                    const char *magic = "RENC";
                    const uint32_t version = 1;

                    std::ostringstream oss(std::ios::binary);

                    // Write header
                    oss.write(magic, 4);
                    oss.write(reinterpret_cast<const char *>(&version), sizeof(version));

                    // Write algorithm
                    uint32_t algo_len = static_cast<uint32_t>(encrypted_data.algorithm.length());
                    oss.write(reinterpret_cast<const char *>(&algo_len), sizeof(algo_len));
                    oss.write(encrypted_data.algorithm.c_str(), algo_len);

                    // Write iterations
                    uint32_t iterations = static_cast<uint32_t>(encrypted_data.iterations);
                    oss.write(reinterpret_cast<const char *>(&iterations), sizeof(iterations));

                    // Write salt
                    uint32_t salt_len = static_cast<uint32_t>(encrypted_data.salt.size());
                    oss.write(reinterpret_cast<const char *>(&salt_len), sizeof(salt_len));
                    oss.write(reinterpret_cast<const char *>(encrypted_data.salt.data()), salt_len);

                    // Write IV
                    uint32_t iv_len = static_cast<uint32_t>(encrypted_data.iv.size());
                    oss.write(reinterpret_cast<const char *>(&iv_len), sizeof(iv_len));
                    oss.write(reinterpret_cast<const char *>(encrypted_data.iv.data()), iv_len);

                    // Write tag
                    uint32_t tag_len = static_cast<uint32_t>(encrypted_data.tag.size());
                    oss.write(reinterpret_cast<const char *>(&tag_len), sizeof(tag_len));
                    oss.write(reinterpret_cast<const char *>(encrypted_data.tag.data()), tag_len);

                    // Write ciphertext
                    uint32_t cipher_len = static_cast<uint32_t>(encrypted_data.ciphertext.size());
                    oss.write(reinterpret_cast<const char *>(&cipher_len), sizeof(cipher_len));
                    oss.write(reinterpret_cast<const char *>(encrypted_data.ciphertext.data()),
                              cipher_len);

                    std::string data = oss.str();
                    output.assign(data.begin(), data.end());

                    return true;

                } catch (const std::exception &e) {
                    SetError("Failed to serialize encrypted data: " + std::string(e.what()));
                    return false;
                }
            }

            bool CryptoUtils::DeserializeEncryptedData(const std::vector<uint8_t> &input,
                                                       EncryptedData &encrypted_data) {
                try {
                    ClearError();

                    if (input.size() < 20) { // Minimum size check
                        SetError("Input data too small");
                        return false;
                    }

                    std::istringstream iss(std::string(input.begin(), input.end()),
                                           std::ios::binary);

                    // Read and verify magic
                    char magic[5] = {0};
                    iss.read(magic, 4);
                    if (std::string(magic) != "RENC") {
                        SetError("Invalid encrypted data format - wrong magic header");
                        return false;
                    }

                    // Read version
                    uint32_t version;
                    iss.read(reinterpret_cast<char *>(&version), sizeof(version));
                    if (version != 1) {
                        SetError("Unsupported encrypted data version: " + std::to_string(version));
                        return false;
                    }

                    // Read algorithm
                    uint32_t algo_len;
                    iss.read(reinterpret_cast<char *>(&algo_len), sizeof(algo_len));
                    if (algo_len > 100) { // Sanity check
                        SetError("Invalid algorithm name length");
                        return false;
                    }
                    encrypted_data.algorithm.resize(algo_len);
                    iss.read(&encrypted_data.algorithm[0], algo_len);

                    // Read iterations
                    uint32_t iterations;
                    iss.read(reinterpret_cast<char *>(&iterations), sizeof(iterations));
                    encrypted_data.iterations = static_cast<int>(iterations);

                    // Read salt
                    uint32_t salt_len;
                    iss.read(reinterpret_cast<char *>(&salt_len), sizeof(salt_len));
                    if (salt_len > 1024) { // Sanity check
                        SetError("Invalid salt length");
                        return false;
                    }
                    encrypted_data.salt.resize(salt_len);
                    iss.read(reinterpret_cast<char *>(encrypted_data.salt.data()), salt_len);

                    // Read IV
                    uint32_t iv_len;
                    iss.read(reinterpret_cast<char *>(&iv_len), sizeof(iv_len));
                    if (iv_len > 1024) { // Sanity check
                        SetError("Invalid IV length");
                        return false;
                    }
                    encrypted_data.iv.resize(iv_len);
                    iss.read(reinterpret_cast<char *>(encrypted_data.iv.data()), iv_len);

                    // Read tag
                    uint32_t tag_len;
                    iss.read(reinterpret_cast<char *>(&tag_len), sizeof(tag_len));
                    if (tag_len > 1024) { // Sanity check
                        SetError("Invalid tag length");
                        return false;
                    }
                    encrypted_data.tag.resize(tag_len);
                    iss.read(reinterpret_cast<char *>(encrypted_data.tag.data()), tag_len);

                    // Read ciphertext
                    uint32_t cipher_len;
                    iss.read(reinterpret_cast<char *>(&cipher_len), sizeof(cipher_len));
                    if (cipher_len > input.size()) { // Sanity check
                        SetError("Invalid ciphertext length");
                        return false;
                    }
                    encrypted_data.ciphertext.resize(cipher_len);
                    iss.read(reinterpret_cast<char *>(encrypted_data.ciphertext.data()),
                             cipher_len);

                    return true;

                } catch (const std::exception &e) {
                    SetError("Failed to deserialize encrypted data: " + std::string(e.what()));
                    return false;
                }
            }

            bool CryptoUtils::GenerateRandomBytes(size_t length, std::vector<uint8_t> &output) {
                try {
                    ClearError();

                    CryptoPP::AutoSeededRandomPool rng;
                    output.resize(length);
                    rng.GenerateBlock(output.data(), length);

                    return true;
                } catch (const CryptoPP::Exception &e) {
                    SetError("Failed to generate random bytes: " + std::string(e.what()));
                    return false;
                }
            }

            bool CryptoUtils::DeriveKey(const std::string &password,
                                        const std::vector<uint8_t> &salt, int iterations,
                                        size_t key_length, std::vector<uint8_t> &derived_key) {
                try {
                    ClearError();

                    if (password.empty() || salt.empty() || key_length == 0) {
                        SetError("Invalid parameters for key derivation");
                        return false;
                    }

                    derived_key.resize(key_length);

                    CryptoPP::PKCS5_PBKDF2_HMAC<CryptoPP::SHA256> pbkdf2;
                    pbkdf2.DeriveKey(derived_key.data(), key_length,
                                     0x00, // unused
                                     reinterpret_cast<const uint8_t *>(password.c_str()),
                                     password.length(), salt.data(), salt.size(), iterations);

                    return true;

                } catch (const CryptoPP::Exception &e) {
                    SetError("Failed to derive key: " + std::string(e.what()));
                    return false;
                }
            }

            bool CryptoUtils::ValidatePassword(const std::string &password) {
                // Minimum password requirements:
                // - At least 8 characters
                // - Contains at least one number or special character
                if (password.length() < 8) {
                    return false;
                }

                bool has_number_or_special = false;
                for (char c : password) {
                    if (std::isdigit(c) || (!std::isalnum(c) && c != ' ')) {
                        has_number_or_special = true;
                        break;
                    }
                }

                return has_number_or_special;
            }

            std::string CryptoUtils::GeneratePassword(size_t length, bool include_symbols) {
                if (length < 12) {
                    length = 12; // Minimum secure length
                }

                const std::string chars =
                    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
                const std::string symbols = "!@#$%^&*()-_=+[]{}|;:,.<>?";

                std::string charset = chars;
                if (include_symbols) {
                    charset += symbols;
                }

                try {
                    CryptoPP::AutoSeededRandomPool rng;
                    std::string password;
                    password.reserve(length);

                    for (size_t i = 0; i < length; ++i) {
                        uint32_t index;
                        rng.GenerateBlock(reinterpret_cast<CryptoPP::byte *>(&index),
                                          sizeof(index));
                        password += charset[index % charset.size()];
                    }

                    return password;

                } catch (const CryptoPP::Exception &) {
                    // Fallback to standard random generator
                    std::random_device rd;
                    std::mt19937 gen(rd());
                    std::uniform_int_distribution<> dis(0, charset.size() - 1);

                    std::string password;
                    password.reserve(length);
                    for (size_t i = 0; i < length; ++i) {
                        password += charset[dis(gen)];
                    }
                    return password;
                }
            }

            const std::string &CryptoUtils::GetLastError() {
                static std::string error = GetThreadSafeError();
                error = GetThreadSafeError(); // Update the static string
                return error;
            }

            bool CryptoUtils::IsCryptoAvailable() {
                try {
                    // Test basic CryptoPP functionality
                    CryptoPP::AutoSeededRandomPool rng;
                    CryptoPP::byte test[16];
                    rng.GenerateBlock(test, sizeof(test));
                    return true;
                } catch (...) {
                    return false;
                }
            }

            std::vector<std::string> CryptoUtils::GetSupportedAlgorithms() {
                return {"AES-256-GCM"};
            }

            void CryptoUtils::SetError(const std::string &error) {
                SetThreadSafeError(error);
            }

            void CryptoUtils::ClearError() {
                ClearThreadSafeError();
            }

            // SecureString implementation
            SecureString::SecureString(const std::string &data) : data_(data) {}

            SecureString::SecureString(const char *data) : data_(data ? data : "") {}

            SecureString::~SecureString() {
                SecureZero();
            }

            SecureString::SecureString(SecureString &&other) noexcept :
                data_(std::move(other.data_)) {
                other.SecureZero();
            }

            SecureString &SecureString::operator=(SecureString &&other) noexcept {
                if (this != &other) {
                    SecureZero();
                    data_ = std::move(other.data_);
                    other.SecureZero();
                }
                return *this;
            }

            void SecureString::SecureZero() {
                if (!data_.empty()) {
                    // Overwrite memory with zeros
                    volatile char *p = const_cast<volatile char *>(data_.data());
                    for (size_t i = 0; i < data_.length(); ++i) {
                        p[i] = 0;
                    }
                    data_.clear();
                }
            }

        } // namespace codec
    }     // namespace utils
} // namespace rayshape
