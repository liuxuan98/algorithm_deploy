/**
 * @file model_packager.cpp
 * @brief Model packaging utility implementation
 * @copyright (c) .
 */

#include "model_packager.h"
#include "../include/utils/file_utils.h"
#include "utils/platform_utils.h"
#include "utils/codec/crypto_utils.h"
#include "utils/codec/auto_crypto.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <zlib.h>

// Simple JSON handling for metadata (avoiding external dependencies)
#include <map>
#include <regex>

namespace rayshape
{
    namespace tools
    {

        bool ModelPackager::CreatePackage(const ModelPackageInfo &package_info,
                                          const std::vector<std::string> &input_files,
                                          const std::string &output_package_path,
                                          bool auto_encrypt) {
            if (input_files.empty()) {
                SetError("No input files specified");
                return false;
            }

            // Validate input files exist
            for (const auto &file : input_files) {
                if (!utils::FileUtils::FileExists(file)) {
                    SetError("Input file does not exist: " + file);
                    return false;
                }
            }

            try {
                std::ostringstream package_stream(std::ios::binary);

                // Determine package magic based on encryption
                const char* magic = auto_encrypt ? "RSME" : PACKAGE_MAGIC;

                // Write package header
                package_stream.write(magic, 4);
                package_stream.write(reinterpret_cast<const char *>(&PACKAGE_VERSION),
                          sizeof(PACKAGE_VERSION));

                // Write metadata
                std::string metadata_content;
                if (!WritePackageMetadata(package_info, metadata_content)) {
                    return false;
                }

                std::vector<uint8_t> compressed_metadata;
                if (!CompressData(metadata_content, compressed_metadata)) {
                    SetError("Failed to compress metadata");
                    return false;
                }

                uint32_t metadata_size = static_cast<uint32_t>(compressed_metadata.size());
                uint32_t metadata_crc =
                    CalculateCRC32(reinterpret_cast<const char *>(compressed_metadata.data()),
                                   compressed_metadata.size());

                package_stream.write(reinterpret_cast<const char *>(&metadata_size), sizeof(metadata_size));
                package_stream.write(reinterpret_cast<const char *>(&metadata_crc), sizeof(metadata_crc));
                package_stream.write(reinterpret_cast<const char *>(compressed_metadata.data()),
                          compressed_metadata.size());

                // Write file count
                uint32_t file_count = static_cast<uint32_t>(input_files.size());
                package_stream.write(reinterpret_cast<const char *>(&file_count), sizeof(file_count));

                // Write files
                for (const auto &file_path : input_files) {
                    std::string file_content;
                    if (!utils::FileUtils::ReadFileContent(file_path, file_content)) {
                        SetError("Failed to read input file: " + file_path);
                        return false;
                    }

                    std::string filename = utils::FileUtils::GetFileName(file_path);

                    // Compress file content
                    std::vector<uint8_t> compressed_content;
                    if (!CompressData(file_content, compressed_content)) {
                        SetError("Failed to compress file: " + filename);
                        return false;
                    }

                    uint32_t name_len = static_cast<uint32_t>(filename.size());
                    uint32_t original_size = static_cast<uint32_t>(file_content.size());
                    uint32_t compressed_size = static_cast<uint32_t>(compressed_content.size());
                    uint32_t file_crc = CalculateCRC32(file_content.data(), file_content.size());

                    // Write file entry
                    package_stream.write(reinterpret_cast<const char *>(&name_len), sizeof(name_len));
                    package_stream.write(filename.c_str(), name_len);
                    package_stream.write(reinterpret_cast<const char *>(&original_size),
                              sizeof(original_size));
                    package_stream.write(reinterpret_cast<const char *>(&compressed_size),
                              sizeof(compressed_size));
                    package_stream.write(reinterpret_cast<const char *>(&file_crc), sizeof(file_crc));
                    package_stream.write(reinterpret_cast<const char *>(compressed_content.data()),
                              compressed_content.size());
                }

                // Get the package data
                std::string package_data = package_stream.str();
                std::vector<uint8_t> final_data;

                // Encrypt the entire package if auto-encryption is enabled
                if (auto_encrypt) {
                    if (!rayshape::utils::codec::AutoCrypto::IsAvailable()) {
                        SetError("Auto-encryption requested but not available");
                        return false;
                    }

                    std::vector<uint8_t> package_bytes(package_data.begin(), package_data.end());

                    if (!rayshape::utils::codec::AutoCrypto::AutoEncrypt(package_bytes, final_data)) {
                        SetError("Failed to auto-encrypt package data: " + rayshape::utils::codec::AutoCrypto::GetLastError());
                        return false;
                    }
                } else {
                    // No encryption, use raw package data
                    final_data.assign(package_data.begin(), package_data.end());
                }

                // Write final data to file
                std::ofstream ofs(output_package_path, std::ios::binary);
                if (!ofs.is_open()) {
                    SetError("Cannot create output package file: " + output_package_path);
                    return false;
                }

                ofs.write(reinterpret_cast<const char*>(final_data.data()), final_data.size());
                return ofs.good();
            } catch (const std::exception &e) {
                SetError("Exception during package creation: " + std::string(e.what()));
                return false;
            }
        }

        bool ModelPackager::ExtractPackage(const std::string &package_path,
                                           const std::string &output_directory) {
            if (!utils::FileUtils::FileExists(package_path)) {
                SetError("Package file does not exist: " + package_path);
                return false;
            }

            if (!utils::FileUtils::DirectoryExists(output_directory)) {
                if (!utils::FileUtils::CreateDirectory(output_directory)) {
                    SetError("Cannot create output directory: " + output_directory);
                    return false;
                }
            }

            try {
                // Read entire package file
                std::vector<uint8_t> package_data;
                if (!utils::FileUtils::ReadBinaryFile(package_path, package_data)) {
                    SetError("Cannot read package file: " + package_path);
                    return false;
                }

                std::vector<uint8_t> decrypted_data;

                // Check if this is an encrypted package
                if (package_data.size() >= 4) {
                    std::string file_magic(package_data.begin(), package_data.begin() + 4);
                    if (file_magic == "RENC") {
                        // This is encrypted data, need to decrypt first
                        // Auto-decryption doesn't require password

                        rayshape::utils::codec::EncryptedData encrypted_data;
                        if (!rayshape::utils::codec::CryptoUtils::DeserializeEncryptedData(package_data, encrypted_data)) {
                            SetError("Failed to deserialize encrypted package: " + rayshape::utils::codec::CryptoUtils::GetLastError());
                            return false;
                        }

                        // TODO: Replace with AutoCrypto
                        if (!rayshape::utils::codec::AutoCrypto::AutoDecrypt(package_data, decrypted_data)) {
                            SetError("Failed to auto-decrypt package: " + rayshape::utils::codec::AutoCrypto::GetLastError());
                            return false;
                        }
                    } else {
                        // Not encrypted data, use as is
                        decrypted_data = package_data;
                    }
                } else {
                    SetError("Invalid package file - too small");
                    return false;
                }

                // Now process the decrypted (or original) package data
                std::istringstream iss(std::string(decrypted_data.begin(), decrypted_data.end()), std::ios::binary);

                // Read and validate header
                char magic[5] = {0};
                iss.read(magic, 4);
                std::string magic_str(magic);
                if (magic_str != PACKAGE_MAGIC && magic_str != "RSME") {
                    SetError("Invalid package format - wrong magic header: " + magic_str);
                    return false;
                }

                uint32_t version;
                iss.read(reinterpret_cast<char *>(&version), sizeof(version));
                if (version != PACKAGE_VERSION) {
                    SetError("Unsupported package version: " + std::to_string(version));
                    return false;
                }

                // Read metadata
                uint32_t metadata_size, metadata_crc;
                iss.read(reinterpret_cast<char *>(&metadata_size), sizeof(metadata_size));
                iss.read(reinterpret_cast<char *>(&metadata_crc), sizeof(metadata_crc));

                std::vector<uint8_t> compressed_metadata(metadata_size);
                iss.read(reinterpret_cast<char *>(compressed_metadata.data()), metadata_size);

                // Verify metadata CRC
                uint32_t calc_crc = CalculateCRC32(
                    reinterpret_cast<const char *>(compressed_metadata.data()), metadata_size);
                if (calc_crc != metadata_crc) {
                    SetError("Metadata CRC mismatch");
                    return false;
                }

                // Decompress metadata
                std::string metadata_content;
                if (!DecompressData(compressed_metadata, metadata_content)) {
                    SetError("Failed to decompress metadata");
                    return false;
                }

                // Save metadata file
                std::string metadata_path =
                    utils::FileUtils::JoinPath(output_directory, METADATA_FILENAME);
                if (!utils::FileUtils::WriteFileContent(metadata_path, metadata_content)) {
                    SetError("Failed to write metadata file");
                    return false;
                }

                // Read file count
                uint32_t file_count;
                iss.read(reinterpret_cast<char *>(&file_count), sizeof(file_count));

                // Extract files
                for (uint32_t i = 0; i < file_count; ++i) {
                    uint32_t name_len;
                    iss.read(reinterpret_cast<char *>(&name_len), sizeof(name_len));

                    std::string filename(name_len, '\0');
                    iss.read(&filename[0], name_len);

                    uint32_t original_size, compressed_size, file_crc;
                    iss.read(reinterpret_cast<char *>(&original_size), sizeof(original_size));
                    iss.read(reinterpret_cast<char *>(&compressed_size), sizeof(compressed_size));
                    iss.read(reinterpret_cast<char *>(&file_crc), sizeof(file_crc));

                    std::vector<uint8_t> compressed_content(compressed_size);
                    iss.read(reinterpret_cast<char *>(compressed_content.data()), compressed_size);

                    // Decompress file content
                    std::string file_content;
                    if (!DecompressData(compressed_content, file_content)) {
                        SetError("Failed to decompress file: " + filename);
                        return false;
                    }

                    // Verify file CRC
                    uint32_t calc_file_crc =
                        CalculateCRC32(file_content.data(), file_content.size());
                    if (calc_file_crc != file_crc) {
                        SetError("File CRC mismatch for: " + filename);
                        return false;
                    }

                    // Write extracted file
                    std::string output_file_path =
                        utils::FileUtils::JoinPath(output_directory, filename);
                    if (!utils::FileUtils::WriteFileContent(output_file_path, file_content)) {
                        SetError("Failed to write extracted file: " + filename);
                        return false;
                    }
                }

                return true;
            } catch (const std::exception &e) {
                SetError("Exception during package extraction: " + std::string(e.what()));
                return false;
            }
        }

        bool ModelPackager::ListPackageContents(const std::string &package_path,
                                                std::vector<std::string> &file_list) {
            if (!utils::FileUtils::FileExists(package_path)) {
                SetError("Package file does not exist: " + package_path);
                return false;
            }

            try {
                std::vector<uint8_t> decrypted_data;
                if (!DecryptAndVerifyPackage(package_path, decrypted_data)) {
                    return false;
                }

                std::istringstream iss(std::string(decrypted_data.begin(), decrypted_data.end()), std::ios::binary);

                // Skip header (already verified)
                iss.seekg(4, std::ios::cur); // magic
                uint32_t version;
                iss.read(reinterpret_cast<char *>(&version), sizeof(version));

                // Skip metadata
                uint32_t metadata_size, metadata_crc;
                iss.read(reinterpret_cast<char *>(&metadata_size), sizeof(metadata_size));
                iss.read(reinterpret_cast<char *>(&metadata_crc), sizeof(metadata_crc));
                iss.seekg(metadata_size, std::ios::cur);

                // Read file count
                uint32_t file_count;
                iss.read(reinterpret_cast<char *>(&file_count), sizeof(file_count));

                file_list.clear();
                file_list.reserve(file_count);

                // Read file names
                for (uint32_t i = 0; i < file_count; ++i) {
                    uint32_t name_len;
                    iss.read(reinterpret_cast<char *>(&name_len), sizeof(name_len));

                    std::string filename(name_len, '\0');
                    iss.read(&filename[0], name_len);
                    file_list.push_back(filename);

                    // Skip file data
                    uint32_t original_size, compressed_size, file_crc;
                    iss.read(reinterpret_cast<char *>(&original_size), sizeof(original_size));
                    iss.read(reinterpret_cast<char *>(&compressed_size), sizeof(compressed_size));
                    iss.read(reinterpret_cast<char *>(&file_crc), sizeof(file_crc));
                    iss.seekg(compressed_size, std::ios::cur);
                }

                return true;
            } catch (const std::exception &e) {
                SetError("Exception during package listing: " + std::string(e.what()));
                return false;
            }
        }

        bool ModelPackager::GetPackageInfo(const std::string &package_path,
                                           ModelPackageInfo &package_info) {
            if (!utils::FileUtils::FileExists(package_path)) {
                SetError("Package file does not exist: " + package_path);
                return false;
            }

            try {
                std::vector<uint8_t> decrypted_data;
                if (!DecryptAndVerifyPackage(package_path, decrypted_data)) {
                    return false;
                }

                std::istringstream iss(std::string(decrypted_data.begin(), decrypted_data.end()), std::ios::binary);

                // Skip header (already verified)
                iss.seekg(4, std::ios::cur); // magic
                uint32_t version;
                iss.read(reinterpret_cast<char *>(&version), sizeof(version));

                // Read metadata
                uint32_t metadata_size, metadata_crc;
                iss.read(reinterpret_cast<char *>(&metadata_size), sizeof(metadata_size));
                iss.read(reinterpret_cast<char *>(&metadata_crc), sizeof(metadata_crc));

                std::vector<uint8_t> compressed_metadata(metadata_size);
                iss.read(reinterpret_cast<char *>(compressed_metadata.data()), metadata_size);

                // Decompress metadata
                std::string metadata_content;
                if (!DecompressData(compressed_metadata, metadata_content)) {
                    SetError("Failed to decompress metadata");
                    return false;
                }

                return ReadPackageMetadata(metadata_content, package_info);
            } catch (const std::exception &e) {
                SetError("Exception during package info reading: " + std::string(e.what()));
                return false;
            }
        }

                bool ModelPackager::ValidatePackage(const std::string &package_path) {
            std::vector<std::string> file_list;
            ModelPackageInfo package_info;

            return ListPackageContents(package_path, file_list)
                   && GetPackageInfo(package_path, package_info);
        }

        bool ModelPackager::WritePackageMetadata(const ModelPackageInfo &package_info,
                                                 std::string &metadata_content) {
            // Simple JSON-like format (avoiding external JSON library dependency)
            std::ostringstream oss;
            oss << "{\n";
            oss << "  \"package_name\": \"" << package_info.package_name << "\",\n";
            oss << "  \"version\": \"" << package_info.version << "\",\n";
            oss << "  \"description\": \"" << package_info.description << "\",\n";
            oss << "  \"model_type\": " << static_cast<int>(package_info.model_type) << ",\n";
            oss << "  \"timestamp\": " << utils::PlatformUtils::GetCurrentTimestamp() << ",\n";
            oss << "  \"platform\": \"" << utils::PlatformUtils::GetPlatformName() << "\",\n";
            oss << "  \"model_files\": [";

            for (size_t i = 0; i < package_info.model_files.size(); ++i) {
                if (i > 0)
                    oss << ", ";
                oss << "\"" << package_info.model_files[i] << "\"";
            }
            oss << "],\n";

            oss << "  \"metadata\": {\n";
            bool first = true;
            for (const auto &kv : package_info.metadata) {
                if (!first)
                    oss << ",\n";
                oss << "    \"" << kv.first << "\": \"" << kv.second << "\"";
                first = false;
            }
            oss << "\n  }\n";
            oss << "}\n";

            metadata_content = oss.str();
            return true;
        }

        bool ModelPackager::ReadPackageMetadata(const std::string &metadata_content,
                                                ModelPackageInfo &package_info) {
            // Simple JSON parsing (basic implementation)
            try {
                // Extract package_name
                std::regex name_regex("\"package_name\"\\s*:\\s*\"([^\"]*)\"");
                std::smatch match;
                if (std::regex_search(metadata_content, match, name_regex)) {
                    package_info.package_name = match[1].str();
                }

                // Extract version
                std::regex version_regex("\"version\"\\s*:\\s*\"([^\"]*)\"");
                if (std::regex_search(metadata_content, match, version_regex)) {
                    package_info.version = match[1].str();
                }

                // Extract description
                std::regex desc_regex("\"description\"\\s*:\\s*\"([^\"]*)\"");
                if (std::regex_search(metadata_content, match, desc_regex)) {
                    package_info.description = match[1].str();
                }

                // Extract model_type
                std::regex type_regex(R"("model_type"\s*:\s*(\d+))");
                if (std::regex_search(metadata_content, match, type_regex)) {
                    package_info.model_type =
                        static_cast<rayshape::ModelType>(std::stoi(match[1].str()));
                }

                return true;
            } catch (const std::exception &e) {
                SetError("Failed to parse metadata: " + std::string(e.what()));
                return false;
            }
        }

        bool ModelPackager::CompressData(const std::string &input, std::vector<uint8_t> &output) {
            uLongf compressed_size = compressBound(static_cast<uLong>(input.size()));
            output.resize(compressed_size);

            int result = compress2(output.data(), &compressed_size,
                                   reinterpret_cast<const Bytef *>(input.data()),
                                   static_cast<uLong>(input.size()), compression_level_);

            if (result != Z_OK) {
                SetError("Compression failed with error: " + std::to_string(result));
                return false;
            }

            output.resize(compressed_size);
            return true;
        }

        bool ModelPackager::DecompressData(const std::vector<uint8_t> &input, std::string &output) {
            // Start with a reasonable buffer size and expand if needed
            uLongf decompressed_size = input.size() * 4;

            for (int attempts = 0; attempts < 3; ++attempts) {
                output.resize(decompressed_size);

                int result = uncompress(reinterpret_cast<Bytef *>(&output[0]), &decompressed_size,
                                        input.data(), static_cast<uLong>(input.size()));

                if (result == Z_OK) {
                    output.resize(decompressed_size);
                    return true;
                } else if (result == Z_BUF_ERROR) {
                    decompressed_size *= 2; // Double the buffer size and try again
                } else {
                    SetError("Decompression failed with error: " + std::to_string(result));
                    return false;
                }
            }

            SetError("Decompression failed after multiple attempts");
            return false;
        }

        uint32_t ModelPackager::CalculateCRC32(const char *data, size_t size) {
            return crc32(0L, reinterpret_cast<const Bytef *>(data), static_cast<uInt>(size));
        }

        void ModelPackager::SetError(const std::string &error) {
            last_error_ = error;
            std::cerr << "ModelPackager Error: " << error << std::endl;
        }

        bool ModelPackager::DecryptAndVerifyPackage(const std::string &package_path,
                                                   std::vector<uint8_t> &decrypted_data) {
            // Read entire package file
            std::vector<uint8_t> package_data;
            if (!utils::FileUtils::ReadBinaryFile(package_path, package_data)) {
                SetError("Cannot read package file: " + package_path);
                return false;
            }

            // Check if this is an encrypted package
            if (package_data.size() >= 4) {
                std::string file_magic(package_data.begin(), package_data.begin() + 4);
                if (file_magic == "RENC") {
                    // This is encrypted data, need to decrypt first
                    // Auto-decrypt the package
                    if (!rayshape::utils::codec::AutoCrypto::AutoDecrypt(package_data, decrypted_data)) {
                        SetError("Failed to auto-decrypt package: " + rayshape::utils::codec::AutoCrypto::GetLastError());
                        return false;
                    }
                } else {
                    // Not encrypted data, use as is
                    decrypted_data = package_data;
                }
            } else {
                SetError("Invalid package file - too small");
                return false;
            }

            // Verify the package header
            if (decrypted_data.size() < 8) {
                SetError("Invalid package data - header too small");
                return false;
            }

            std::string magic_str(decrypted_data.begin(), decrypted_data.begin() + 4);
            if (magic_str != PACKAGE_MAGIC && magic_str != "RSME") {
                SetError("Invalid package format - wrong magic header: " + magic_str);
                return false;
            }

            return true;
        }

    } // namespace tools
} // namespace rayshape
