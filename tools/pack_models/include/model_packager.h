/**
 * @file model_packager.h
 * @brief Model packaging utility for creating model packages
 * @copyright (c) .
 */

#ifndef PACK_MODELS_MODEL_PACKAGER_H_
#define PACK_MODELS_MODEL_PACKAGER_H_

#include <string>
#include <vector>
#include <map>
#include <memory>

#include "base/common.h"

namespace rayshape
{
    namespace tools
    {

        /**
         * @brief Model package information structure
         */
        struct ModelPackageInfo {
            std::string package_name;
            std::string version;
            std::string description;
            rayshape::ModelType model_type;
            std::vector<std::string> model_files;
            std::map<std::string, std::string> metadata;

            ModelPackageInfo() : model_type(rayshape::ModelType::NONE) {}
        };

        /**
         * @brief Model packaging utility class
         *
         * This class provides functionality to package multiple models and related files
         * into a single compressed archive with metadata.
         */
        class ModelPackager {
        public:
            ModelPackager() = default;
            ~ModelPackager() = default;

            /**
             * @brief Create a model package from multiple files
             * @param package_info Package information
             * @param input_files List of input files to package
             * @param output_package_path Output package file path
             * @param auto_encrypt Enable automatic encryption
             * @return true if successful, false otherwise
             */
            bool CreatePackage(const ModelPackageInfo &package_info,
                               const std::vector<std::string> &input_files,
                               const std::string &output_package_path,
                               bool auto_encrypt = false);

            /**
             * @brief Extract a model package (auto-detects and handles encryption)
             * @param package_path Path to package file
             * @param output_directory Output directory for extracted files
             * @return true if successful, false otherwise
             */
            bool ExtractPackage(const std::string &package_path,
                                const std::string &output_directory);

            /**
             * @brief List contents of a model package (auto-detects and handles encryption)
             * @param package_path Path to package file
             * @param file_list Output list of files in package
             * @return true if successful, false otherwise
             */
            bool ListPackageContents(const std::string &package_path,
                                     std::vector<std::string> &file_list);

            /**
             * @brief Get package information (auto-detects and handles encryption)
             * @param package_path Path to package file
             * @param package_info Output package information
             * @return true if successful, false otherwise
             */
            bool GetPackageInfo(const std::string &package_path, ModelPackageInfo &package_info);

            /**
             * @brief Validate package integrity (auto-detects and handles encryption)
             * @param package_path Path to package file
             * @return true if valid, false otherwise
             */
            bool ValidatePackage(const std::string &package_path);

            /**
             * @brief Get last error message
             * @return Error message string
             */
            const std::string &GetLastError() const {
                return last_error_;
            }

            /**
             * @brief Set compression level (0-9, where 9 is maximum compression)
             * @param level Compression level
             */
            void SetCompressionLevel(int level) {
                compression_level_ = level;
            }

        private:
            /**
             * @brief Write package metadata
             * @param package_info Package information
             * @param metadata_content Output metadata content
             * @return true if successful, false otherwise
             */
            bool WritePackageMetadata(const ModelPackageInfo &package_info,
                                      std::string &metadata_content);

            /**
             * @brief Read package metadata
             * @param metadata_content Input metadata content
             * @param package_info Output package information
             * @return true if successful, false otherwise
             */
            bool ReadPackageMetadata(const std::string &metadata_content,
                                     ModelPackageInfo &package_info);

            /**
             * @brief Compress data using zlib
             * @param input Input data
             * @param output Compressed output data
             * @return true if successful, false otherwise
             */
            bool CompressData(const std::string &input, std::vector<uint8_t> &output);

            /**
             * @brief Decompress data using zlib
             * @param input Compressed input data
             * @param output Decompressed output data
             * @return true if successful, false otherwise
             */
            bool DecompressData(const std::vector<uint8_t> &input, std::string &output);

            /**
             * @brief Calculate CRC32 checksum
             * @param data Input data
             * @param size Data size
             * @return CRC32 checksum
             */
            uint32_t CalculateCRC32(const char *data, size_t size);

            /**
             * @brief Set last error message
             * @param error Error message
             */
            void SetError(const std::string &error);

            /**
             * @brief Auto-decrypt and verify package data
             * @param package_path Path to package file
             * @param decrypted_data Output decrypted package data
             * @return true if successful, false otherwise
             */
            bool DecryptAndVerifyPackage(const std::string &package_path,
                                        std::vector<uint8_t> &decrypted_data);

        private:
            std::string last_error_;
            int compression_level_ = 6; // Default compression level

            // Package format constants
            static constexpr const char *PACKAGE_MAGIC = "RSMP"; // RayShape Model Package
            static constexpr uint32_t PACKAGE_VERSION = 1;
            static constexpr const char *METADATA_FILENAME = "package_info.json";
        };

    } // namespace tools
} // namespace rayshape

#endif // PACK_MODELS_MODEL_PACKAGER_H_
