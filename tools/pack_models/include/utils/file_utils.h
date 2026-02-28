/**
 * @file file_utils.h
 * @brief Cross-platform file utility functions
 * @copyright (c) .
 */

#ifndef PACK_MODELS_UTILS_FILE_UTILS_H_
#define PACK_MODELS_UTILS_FILE_UTILS_H_

#include <string>
#include <vector>

namespace rayshape
{
    namespace tools
    {
        namespace utils
        {

            /**
             * @brief File utility functions
             */
            class FileUtils {
            public:
                /**
                 * @brief Check if file exists
                 * @param file_path Path to file
                 * @return true if exists, false otherwise
                 */
                static bool FileExists(const std::string &file_path);

                /**
                 * @brief Check if directory exists
                 * @param dir_path Path to directory
                 * @return true if exists, false otherwise
                 */
                static bool DirectoryExists(const std::string &dir_path);

                /**
                 * @brief Create directory recursively
                 * @param dir_path Path to directory
                 * @return true if successful, false otherwise
                 */
                static bool CreateDirectory(const std::string &dir_path);

                /**
                 * @brief Get file size
                 * @param file_path Path to file
                 * @return File size in bytes, -1 on error
                 */
                static long long GetFileSize(const std::string &file_path);

                /**
                 * @brief Read entire file content into string
                 * @param file_path Path to file
                 * @param content Output content
                 * @return true if successful, false otherwise
                 */
                static bool ReadFileContent(const std::string &file_path, std::string &content);

                /**
                 * @brief Write string content to file
                 * @param file_path Path to file
                 * @param content Content to write
                 * @return true if successful, false otherwise
                 */
                static bool WriteFileContent(const std::string &file_path,
                                             const std::string &content);

                /**
                 * @brief Read binary file content into vector
                 * @param file_path Path to file
                 * @param data Output data
                 * @return true if successful, false otherwise
                 */
                static bool ReadBinaryFile(const std::string &file_path,
                                           std::vector<uint8_t> &data);

                /**
                 * @brief Write binary data to file
                 * @param file_path Path to file
                 * @param data Data to write
                 * @return true if successful, false otherwise
                 */
                static bool WriteBinaryFile(const std::string &file_path,
                                            const std::vector<uint8_t> &data);

                /**
                 * @brief Get file extension (without dot)
                 * @param file_path Path to file
                 * @return File extension
                 */
                static std::string GetFileExtension(const std::string &file_path);

                /**
                 * @brief Get file name from path
                 * @param file_path Path to file
                 * @return File name
                 */
                static std::string GetFileName(const std::string &file_path);

                /**
                 * @brief Get directory path from file path
                 * @param file_path Path to file
                 * @return Directory path
                 */
                static std::string GetDirectoryPath(const std::string &file_path);

                /**
                 * @brief Join two paths
                 * @param path1 First path
                 * @param path2 Second path
                 * @return Joined path
                 */
                static std::string JoinPath(const std::string &path1, const std::string &path2);

                /**
                 * @brief Normalize path
                 * @param path Path to normalize
                 * @return Normalized path
                 */
                static std::string NormalizePath(const std::string &path);

                /**
                 * @brief Get system temp directory
                 * @return Temp directory path
                 */
                static std::string GetTempDirectory();

                /**
                 * @brief Generate temporary file path
                 * @param prefix File prefix
                 * @param extension File extension (default: .tmp)
                 * @return Temporary file path
                 */
                static std::string GenerateTempFilePath(const std::string &prefix,
                                                        const std::string &extension = ".tmp");

                /**
                 * @brief Delete file
                 * @param file_path Path to file
                 * @return true if successful, false otherwise
                 */
                static bool DeleteFile(const std::string &file_path);

                /**
                 * @brief Copy file
                 * @param source_path Source file path
                 * @param dest_path Destination file path
                 * @return true if successful, false otherwise
                 */
                static bool CopyFile(const std::string &source_path, const std::string &dest_path);

                /**
                 * @brief List files in directory
                 * @param dir_path Directory path
                 * @param files Output file list
                 * @param recursive Whether to search recursively
                 * @return true if successful, false otherwise
                 */
                static bool ListFiles(const std::string &dir_path, std::vector<std::string> &files,
                                      bool recursive = false);

            private:
                FileUtils() = delete; // Static class
            };

        } // namespace utils
    } // namespace tools
} // namespace rayshape

#endif // PACK_MODELS_UTILS_FILE_UTILS_H_