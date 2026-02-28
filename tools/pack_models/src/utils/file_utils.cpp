/**
 * @file file_utils.cpp
 * @brief Cross-platform file utility functions implementation
 * @copyright (c) .
 */

#include "utils/file_utils.h"
#include "utils/platform_utils.h"

#include <fstream>
#include <filesystem>
#include <random>
#include <sstream>
#include <iomanip>

namespace rayshape
{
    namespace tools
    {
        namespace utils
        {

            bool FileUtils::FileExists(const std::string &file_path) {
                try {
                    return std::filesystem::exists(file_path)
                           && std::filesystem::is_regular_file(file_path);
                } catch (const std::exception &) {
                    return false;
                }
            }

            bool FileUtils::DirectoryExists(const std::string &dir_path) {
                try {
                    return std::filesystem::exists(dir_path)
                           && std::filesystem::is_directory(dir_path);
                } catch (const std::exception &) {
                    return false;
                }
            }

            bool FileUtils::CreateDirectory(const std::string &dir_path) {
                try {
                    return std::filesystem::create_directories(dir_path);
                } catch (const std::exception &) {
                    return false;
                }
            }

            long long FileUtils::GetFileSize(const std::string &file_path) {
                try {
                    if (!FileExists(file_path)) {
                        return -1;
                    }
                    return static_cast<long long>(std::filesystem::file_size(file_path));
                } catch (const std::exception &) {
                    return -1;
                }
            }

            bool FileUtils::ReadFileContent(const std::string &file_path, std::string &content) {
                try {
                    std::ifstream ifs(file_path, std::ios::binary);
                    if (!ifs.is_open()) {
                        return false;
                    }

                    ifs.seekg(0, std::ios::end);
                    size_t size = ifs.tellg();
                    ifs.seekg(0, std::ios::beg);

                    content.resize(size);
                    ifs.read(&content[0], size);
                    return ifs.good();
                } catch (const std::exception &) {
                    return false;
                }
            }

            bool FileUtils::WriteFileContent(const std::string &file_path,
                                             const std::string &content) {
                try {
                    // Create directory if it doesn't exist
                    std::string dir_path = GetDirectoryPath(file_path);
                    if (!dir_path.empty() && !DirectoryExists(dir_path)) {
                        if (!CreateDirectory(dir_path)) {
                            return false;
                        }
                    }

                    std::ofstream ofs(file_path, std::ios::binary);
                    if (!ofs.is_open()) {
                        return false;
                    }

                    ofs.write(content.data(), content.size());
                    return ofs.good();
                } catch (const std::exception &) {
                    return false;
                }
            }

            bool FileUtils::ReadBinaryFile(const std::string &file_path,
                                           std::vector<uint8_t> &data) {
                try {
                    std::ifstream ifs(file_path, std::ios::binary);
                    if (!ifs.is_open()) {
                        return false;
                    }

                    ifs.seekg(0, std::ios::end);
                    size_t size = ifs.tellg();
                    ifs.seekg(0, std::ios::beg);

                    data.resize(size);
                    ifs.read(reinterpret_cast<char *>(data.data()), size);
                    return ifs.good();
                } catch (const std::exception &) {
                    return false;
                }
            }

            bool FileUtils::WriteBinaryFile(const std::string &file_path,
                                            const std::vector<uint8_t> &data) {
                try {
                    // Create directory if it doesn't exist
                    std::string dir_path = GetDirectoryPath(file_path);
                    if (!dir_path.empty() && !DirectoryExists(dir_path)) {
                        if (!CreateDirectory(dir_path)) {
                            return false;
                        }
                    }

                    std::ofstream ofs(file_path, std::ios::binary);
                    if (!ofs.is_open()) {
                        return false;
                    }

                    ofs.write(reinterpret_cast<const char *>(data.data()), data.size());
                    return ofs.good();
                } catch (const std::exception &) {
                    return false;
                }
            }

            std::string FileUtils::GetFileExtension(const std::string &file_path) {
                try {
                    std::filesystem::path path(file_path);
                    std::string ext = path.extension().string();
                    if (!ext.empty() && ext[0] == '.') {
                        return ext.substr(1); // Remove the dot
                    }
                    return ext;
                } catch (const std::exception &) {
                    return "";
                }
            }

            std::string FileUtils::GetFileName(const std::string &file_path) {
                try {
                    std::filesystem::path path(file_path);
                    return path.filename().string();
                } catch (const std::exception &) {
                    return "";
                }
            }

            std::string FileUtils::GetDirectoryPath(const std::string &file_path) {
                try {
                    std::filesystem::path path(file_path);
                    return path.parent_path().string();
                } catch (const std::exception &) {
                    return "";
                }
            }

            std::string FileUtils::JoinPath(const std::string &path1, const std::string &path2) {
                try {
                    std::filesystem::path p1(path1);
                    std::filesystem::path p2(path2);
                    return (p1 / p2).string();
                } catch (const std::exception &) {
                    return path1 + PlatformUtils::GetPathSeparator() + path2;
                }
            }

            std::string FileUtils::NormalizePath(const std::string &path) {
                try {
                    std::filesystem::path p(path);
                    return std::filesystem::canonical(p).string();
                } catch (const std::exception &) {
                    return path;
                }
            }

            std::string FileUtils::GetTempDirectory() {
                try {
                    return std::filesystem::temp_directory_path().string();
                } catch (const std::exception &) {
                    // Fallback to platform-specific temp directory
                    std::string temp_dir = PlatformUtils::GetEnvironmentVariable("TEMP");
                    if (temp_dir.empty()) {
                        temp_dir = PlatformUtils::GetEnvironmentVariable("TMP");
                    }
                    if (temp_dir.empty()) {
                        temp_dir = PlatformUtils::IsWindows() ? "C:\\temp" : "/tmp";
                    }
                    return temp_dir;
                }
            }

            std::string FileUtils::GenerateTempFilePath(const std::string &prefix,
                                                        const std::string &extension) {
                std::string temp_dir = GetTempDirectory();

                // Generate random suffix
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> dis(0, 15);

                std::stringstream ss;
                ss << prefix << "_";
                for (int i = 0; i < 8; ++i) {
                    ss << std::hex << dis(gen);
                }
                ss << extension;

                return JoinPath(temp_dir, ss.str());
            }

            bool FileUtils::DeleteFile(const std::string &file_path) {
                try {
                    return std::filesystem::remove(file_path);
                } catch (const std::exception &) {
                    return false;
                }
            }

            bool FileUtils::CopyFile(const std::string &source_path, const std::string &dest_path) {
                try {
                    // Create destination directory if it doesn't exist
                    std::string dest_dir = GetDirectoryPath(dest_path);
                    if (!dest_dir.empty() && !DirectoryExists(dest_dir)) {
                        if (!CreateDirectory(dest_dir)) {
                            return false;
                        }
                    }

                    std::filesystem::copy_file(source_path, dest_path,
                                               std::filesystem::copy_options::overwrite_existing);
                    return true;
                } catch (const std::exception &) {
                    return false;
                }
            }

            bool FileUtils::ListFiles(const std::string &dir_path, std::vector<std::string> &files,
                                      bool recursive) {
                try {
                    files.clear();

                    if (recursive) {
                        for (const auto &entry :
                             std::filesystem::recursive_directory_iterator(dir_path)) {
                            if (entry.is_regular_file()) {
                                files.push_back(entry.path().string());
                            }
                        }
                    } else {
                        for (const auto &entry : std::filesystem::directory_iterator(dir_path)) {
                            if (entry.is_regular_file()) {
                                files.push_back(entry.path().string());
                            }
                        }
                    }

                    return true;
                } catch (const std::exception &) {
                    return false;
                }
            }

        } // namespace utils
    } // namespace tools
} // namespace rayshape
