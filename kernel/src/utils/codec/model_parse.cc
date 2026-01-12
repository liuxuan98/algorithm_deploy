/**
 * @file model_parse.cc
 * @brief Model parsing implementation
 * @copyright (c) 2025 Shenzhen RayShape Medical Technology Co., Ltd.
 */

#include "utils/codec/model_parse.h"
#include "utils/codec/model_serialize.h" // CRITICAL: Include model serialization registration
#include "base/logger.h"

#ifdef ENABLE_CRYPTOPP
#include "utils/codec/auto_crypto.h"
#endif

#include <fstream>
#include <sstream>

namespace rayshape
{

    std::unique_ptr<Model> ParseModel(const std::string &filename) {
        try {
            // First, read the entire file into memory
            std::ifstream ifs(filename, std::ios::binary);
            if (!ifs.is_open()) {
                RS_LOGE("Cannot open model file: %s\n", filename.c_str());
                return nullptr;
            }

            // Get file size
            ifs.seekg(0, std::ios::end);
            std::size_t file_size = ifs.tellg();
            ifs.seekg(0, std::ios::beg);

            // Read entire file into buffer
            std::vector<uint8_t> file_data(file_size);
            ifs.read(reinterpret_cast<char *>(file_data.data()), file_size);
            ifs.close();

            if (!ifs) {
                RS_LOGE("Failed to read model file: %s\n", filename.c_str());
                return nullptr;
            }

#ifdef ENABLE_CRYPTOPP
            // Check if this is an encrypted model and decrypt if necessary
            std::vector<uint8_t> model_data;
            if (rayshape::utils::codec::AutoCrypto::IsAutoEncrypted(file_data)) {
                RS_LOGD("Detected encrypted model, decrypting: %s\n", filename.c_str());
                if (!rayshape::utils::codec::AutoCrypto::AutoDecrypt(file_data, model_data)) {
                    RS_LOGE("Failed to decrypt model file %s: %s\n", filename.c_str(),
                            rayshape::utils::codec::AutoCrypto::GetLastError().c_str());
                    return nullptr;
                }
                RS_LOGD("Successfully decrypted model: %s\n", filename.c_str());
            } else {
                // Not encrypted, use original data
                model_data = std::move(file_data);
            }
#else
            // No encryption support, use original data
            std::vector<uint8_t> model_data = std::move(file_data);

            // Check if this might be encrypted
            if (model_data.size() >= 4) {
                std::string magic_str(model_data.begin(), model_data.begin() + 4);
                if (magic_str == "RENC" || magic_str == "RAEC") {
                    RS_LOGE(
                        "Model file %s appears to be encrypted but encryption support is not available\n",
                        filename.c_str());
                    return nullptr;
                }
            }
#endif

            // Parse the (possibly decrypted) model data
            std::istringstream iss(std::string(model_data.begin(), model_data.end()),
                                   std::ios::binary);
            ModelCodec model_codec;
            cereal::BinaryInputArchive ia(iss);
            ia(model_codec);

            if (model_codec.model_ == nullptr) {
                RS_LOGE("Parsed model is null from file: %s\n", filename.c_str());
                return nullptr;
            }

            RS_LOGD("Successfully parsed model from file: %s\n", filename.c_str());
            return std::move(model_codec.model_);
        } catch (const std::exception &e) {
            RS_LOGE("Exception parsing model from file %s: %s\n", filename.c_str(), e.what());
            return nullptr;
        }
    }

    std::unique_ptr<Model> ParseModel(const char *model_buf, std::size_t size) {
        if (model_buf == nullptr || size == 0) {
            RS_LOGE("Invalid model buffer parameters\n");
            return nullptr;
        }

        try {
            // Convert to vector for easier handling
            std::vector<uint8_t> buffer_data(model_buf, model_buf + size);

#ifdef ENABLE_CRYPTOPP
            // Check if this is an encrypted model and decrypt if necessary
            std::vector<uint8_t> model_data;
            if (rayshape::utils::codec::AutoCrypto::IsAutoEncrypted(buffer_data)) {
                RS_LOGD("Detected encrypted model in memory buffer, decrypting\n");
                if (!rayshape::utils::codec::AutoCrypto::AutoDecrypt(buffer_data, model_data)) {
                    RS_LOGE("Failed to decrypt model from memory buffer: %s\n",
                            rayshape::utils::codec::AutoCrypto::GetLastError().c_str());
                    return nullptr;
                }
                RS_LOGD("Successfully decrypted model from memory buffer\n");
            } else {
                // Not encrypted, use original data
                model_data = std::move(buffer_data);
            }
#else
            // No encryption support, use original data
            std::vector<uint8_t> model_data = std::move(buffer_data);

            // Check if this might be encrypted
            if (model_data.size() >= 4) {
                std::string magic_str(model_data.begin(), model_data.begin() + 4);
                if (magic_str == "RENC" || magic_str == "RAEC") {
                    RS_LOGE(
                        "Memory buffer appears to contain encrypted model but encryption support is not available\n");
                    return nullptr;
                }
            }
#endif

            // Parse the (possibly decrypted) model data
            std::istringstream iss(std::string(model_data.begin(), model_data.end()),
                                   std::ios::binary);
            ModelCodec model_codec;
            cereal::BinaryInputArchive ia(iss);
            ia(model_codec);

            if (model_codec.model_ == nullptr) {
                RS_LOGE("Parsed model is null from memory buffer\n");
                return nullptr;
            }

            RS_LOGD("Successfully parsed model from memory buffer (size: %zu)\n", size);
            return std::move(model_codec.model_);
        } catch (const std::exception &e) {
            RS_LOGE("Exception parsing model from memory buffer: %s\n", e.what());
            return nullptr;
        }
    }

} // namespace rayshape
