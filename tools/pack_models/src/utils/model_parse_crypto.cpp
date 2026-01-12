/**
 * @file model_parse_crypto.cpp
 * @brief Model parsing with encryption support implementation
 * @copyright (c) 2025 Shenzhen RayShape Medical Technology Co., Ltd.
 */

#include "utils/model_parse_crypto.h"
#include "utils/codec/auto_crypto.h"
#include "utils/codec/crypto_utils.h"
#include "utils/file_utils.h"

// Include cereal and model serialization
#include "utils/codec/model_serialize.h"
#include <cereal/archives/binary.hpp>

#include <fstream>
#include <sstream>
#include <iostream>
#include <mutex>

namespace rayshape
{
    namespace tools
    {
        namespace utils
        {

            // Thread-safe error handling
            namespace {
                std::mutex error_mutex;
                std::string last_error;

                void SetThreadSafeError(const std::string &error) {
                    std::lock_guard<std::mutex> lock(error_mutex);
                    last_error = error;
                    std::cerr << "[ModelParserCrypto Error] " << error << std::endl;
                }

                std::string GetThreadSafeError() {
                    std::lock_guard<std::mutex> lock(error_mutex);
                    return last_error;
                }

                void ClearThreadSafeError() {
                    std::lock_guard<std::mutex> lock(error_mutex);
                    last_error.clear();
                }
            }

            std::unique_ptr<rayshape::Model> ModelParserCrypto::ParseModel(const std::string &filename,
                                                                          const std::string &password) {
                ClearError();

                if (filename.empty()) {
                    SetError("Filename is empty");
                    return nullptr;
                }

                if (!FileUtils::FileExists(filename)) {
                    SetError("Model file does not exist: " + filename);
                    return nullptr;
                }

                // Read file data
                std::vector<uint8_t> file_data;
                if (!FileUtils::ReadBinaryFile(filename, file_data)) {
                    SetError("Failed to read model file: " + filename);
                    return nullptr;
                }

                return ParseModel(file_data, password);
            }

            std::unique_ptr<rayshape::Model> ModelParserCrypto::ParseModel(const char *model_buf, std::size_t size,
                                                                          const std::string &password) {
                ClearError();

                if (model_buf == nullptr || size == 0) {
                    SetError("Invalid model buffer parameters");
                    return nullptr;
                }

                std::vector<uint8_t> model_data(model_buf, model_buf + size);
                return ParseModel(model_data, password);
            }

            std::unique_ptr<rayshape::Model> ModelParserCrypto::ParseModel(const std::vector<uint8_t> &model_data,
                                                                          const std::string &password) {
                ClearError();

                if (model_data.empty()) {
                    SetError("Model data is empty");
                    return nullptr;
                }

                try {
                    std::vector<uint8_t> decrypted_data;

                    // Check if data is encrypted
                    if (IsDataEncrypted(reinterpret_cast<const char*>(model_data.data()), model_data.size())) {
                        // Data is encrypted, need password
                        if (password.empty()) {
                            SetError("Model data is encrypted but no password provided");
                            return nullptr;
                        }

                        if (!rayshape::utils::codec::CryptoUtils::IsCryptoAvailable()) {
                            SetError("Model data is encrypted but CryptoPP library is not available");
                            return nullptr;
                        }

                        // Deserialize encrypted data
                        rayshape::utils::codec::EncryptedData encrypted_data;
                        if (!rayshape::utils::codec::CryptoUtils::DeserializeEncryptedData(model_data, encrypted_data)) {
                            SetError("Failed to deserialize encrypted model data: " + rayshape::utils::codec::CryptoUtils::GetLastError());
                            return nullptr;
                        }

                        // Decrypt the data
                        if (!rayshape::utils::codec::CryptoUtils::DecryptData(encrypted_data, password, decrypted_data)) {
                            SetError("Failed to decrypt model data: " + rayshape::utils::codec::CryptoUtils::GetLastError());
                            return nullptr;
                        }
                    } else {
                        // Data is not encrypted, use as-is
                        if (!password.empty()) {
                            SetError("Password provided but model data is not encrypted");
                            return nullptr;
                        }
                        decrypted_data = model_data;
                    }

                    // Parse the decrypted (or original) data
                    return ParseDecryptedData(decrypted_data);

                } catch (const std::exception &e) {
                    SetError("Exception during model parsing: " + std::string(e.what()));
                    return nullptr;
                }
            }

            bool ModelParserCrypto::IsDataEncrypted(const char* data, std::size_t size) {
                if (data == nullptr || size < 4) {
                    return false;
                }

                // Check for encrypted data magic "RENC"
                return std::string(data, 4) == "RENC";
            }

            bool ModelParserCrypto::IsFileEncrypted(const std::string &filename) {
                if (!FileUtils::FileExists(filename)) {
                    return false;
                }

                std::ifstream ifs(filename, std::ios::binary);
                if (!ifs.is_open()) {
                    return false;
                }

                char magic[4];
                ifs.read(magic, 4);
                if (!ifs.good()) {
                    return false;
                }

                return std::string(magic, 4) == "RENC";
            }

            const std::string& ModelParserCrypto::GetLastError() {
                static std::string error = GetThreadSafeError();
                return error;
            }

            std::unique_ptr<rayshape::Model> ModelParserCrypto::ParseDecryptedData(const std::vector<uint8_t> &decrypted_data) {
                try {
                    std::istringstream iss(std::string(decrypted_data.begin(), decrypted_data.end()), std::ios::binary);

                    rayshape::ModelCodec model_codec;
                    cereal::BinaryInputArchive ia(iss);
                    ia(model_codec);

                    if (model_codec.model_ == nullptr) {
                        SetError("Parsed model is null");
                        return nullptr;
                    }

                    return std::move(model_codec.model_);

                } catch (const std::exception &e) {
                    SetError("Exception during cereal deserialization: " + std::string(e.what()));
                    return nullptr;
                }
            }

            void ModelParserCrypto::SetError(const std::string &error) {
                SetThreadSafeError(error);
            }

            void ModelParserCrypto::ClearError() {
                ClearThreadSafeError();
            }

        } // namespace utils
    } // namespace tools
} // namespace rayshape
