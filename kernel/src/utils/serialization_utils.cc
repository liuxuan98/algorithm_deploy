#include "utils/serialization_utils.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <zlib.h>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <cerrno>
#endif

namespace rayshape
{
    namespace utils
    {

        const char SerializationUtils::MAGIC_HEADER[4] = {'P', 'K', 'G', '1'};

        std::string SerializationUtils::ExtractFilename(const std::string &file_path) {
            // Find the last occurrence of path separator
            size_t pos = file_path.find_last_of("/\\");
            if (pos == std::string::npos) {
                return file_path; // No path separator found, return the whole string
            }
            return file_path.substr(pos + 1);
        }

        std::string SerializationUtils::JoinPath(const std::string &dir,
                                                 const std::string &filename) {
            if (dir.empty()) {
                return filename;
            }
            if (filename.empty()) {
                return dir;
            }

            // Ensure directory ends with separator
            std::string result = dir;
            char separator = '/';
#ifdef _WIN32
            separator = '\\';
#endif

            if (!result.empty() && result[result.size() - 1] != '/'
                && result[result.size() - 1] != '\\') {
                result += separator;
            }

            return result + filename;
        }

        bool SerializationUtils::CreateDirectories(const std::string &dir_path) {
            if (dir_path.empty()) {
                return false;
            }

#ifdef _WIN32
            return _mkdir(dir_path.c_str()) == 0 || errno == EEXIST;
#else
            return mkdir(dir_path.c_str(), 0755) == 0 || errno == EEXIST;
#endif
        }

        bool SerializationUtils::SerializeFiles(const std::vector<std::string> &input_files,
                                                const std::string &output_file) {
            std::ofstream ofs(output_file.c_str(), std::ios::binary);
            if (!ofs) {
                std::cerr << "Failed to open output file: " << output_file << '\n';
                return false;
            }

            // Write magic header
            ofs.write(MAGIC_HEADER, 4);

            uint64_t file_count = input_files.size();
            ofs.write(reinterpret_cast<const char *>(&file_count), sizeof(file_count));

            for (const auto &file_path : input_files) {
                std::ifstream ifs(file_path.c_str(), std::ios::binary);
                if (!ifs) {
                    std::cerr << "Cannot open file: " << file_path << '\n';
                    continue;
                }

                ifs.seekg(0, std::ios::end);
                uint64_t file_size = static_cast<uint64_t>(ifs.tellg());
                ifs.seekg(0);

                std::vector<char> buffer(file_size);
                ifs.read(&buffer[0], file_size);

                std::string filename = ExtractFilename(file_path);
                uint64_t name_len = filename.size();

                uint32_t crc = CalculateCrc32(&buffer[0], file_size);

                ofs.write(reinterpret_cast<const char *>(&name_len), sizeof(name_len));
                ofs.write(filename.c_str(), name_len);
                ofs.write(reinterpret_cast<const char *>(&file_size), sizeof(file_size));
                ofs.write(&buffer[0], file_size);
                ofs.write(reinterpret_cast<const char *>(&crc), sizeof(crc));
            }

            return true;
        }

        bool SerializationUtils::DeSerializeFiles(const std::string &input_file,
                                                  const std::string &output_dir) {
            std::ifstream ifs(input_file.c_str(), std::ios::binary);
            if (!ifs) {
                std::cerr << "Failed to open input file: " << input_file << '\n';
                return false;
            }

            char magic[4];
            ifs.read(magic, 4);
            if (std::string(magic, 4) != "PKG1") {
                std::cerr << "Invalid magic header!" << '\n';
                return false;
            }

            uint64_t file_count = 0;
            ifs.read(reinterpret_cast<char *>(&file_count), sizeof(file_count));

            CreateDirectories(output_dir);

            for (uint64_t i = 0; i < file_count; ++i) {
                uint64_t name_len = 0;
                ifs.read(reinterpret_cast<char *>(&name_len), sizeof(name_len));

                std::string filename(name_len, '\0');
                ifs.read(&filename[0], name_len);

                uint64_t file_size = 0;
                ifs.read(reinterpret_cast<char *>(&file_size), sizeof(file_size));

                std::vector<char> buffer(file_size);
                ifs.read(&buffer[0], file_size);

                uint32_t stored_crc = 0;
                ifs.read(reinterpret_cast<char *>(&stored_crc), sizeof(stored_crc));

                uint32_t calc_crc = CalculateCrc32(&buffer[0], file_size);

                if (stored_crc != calc_crc) {
                    std::cerr << "Checksum mismatch for: " << filename << '\n';
                    continue;
                }

                std::string output_path = JoinPath(output_dir, filename);
                std::ofstream ofs(output_path.c_str(), std::ios::binary);
                if (!ofs) {
                    std::cerr << "Failed to write: " << output_path << '\n';
                    continue;
                }

                ofs.write(&buffer[0], file_size);
                std::cout << "Unpacked: " << filename << '\n';
            }

            return true;
        }

        bool SerializationUtils::SerializeToStream(
            const std::map<std::string, std::string> &data_map, std::ostream &output_stream) {
            // Write magic header
            output_stream.write(MAGIC_HEADER, 4);

            uint64_t file_count = data_map.size();
            output_stream.write(reinterpret_cast<const char *>(&file_count), sizeof(file_count));

            for (const auto &it : data_map) {
                const std::string &filename = it.first;
                const std::string &content = it.second;

                uint64_t name_len = filename.size();
                uint64_t file_size = content.size();

                uint32_t crc = CalculateCrc32(content.data(), file_size);

                output_stream.write(reinterpret_cast<const char *>(&name_len), sizeof(name_len));
                output_stream.write(filename.c_str(), name_len);
                output_stream.write(reinterpret_cast<const char *>(&file_size), sizeof(file_size));
                output_stream.write(content.data(), file_size);
                output_stream.write(reinterpret_cast<const char *>(&crc), sizeof(crc));
            }

            return output_stream.good();
        }

        bool SerializationUtils::DeserializeFromStream(
            std::istream &input_stream, std::map<std::string, std::string> &data_map) {
            char magic[4];
            input_stream.read(magic, 4);
            if (std::string(magic, 4) != "PKG1") {
                std::cerr << "Invalid magic header!" << '\n';
                return false;
            }

            uint64_t file_count = 0;
            input_stream.read(reinterpret_cast<char *>(&file_count), sizeof(file_count));

            data_map.clear();

            for (uint64_t i = 0; i < file_count; ++i) {
                uint64_t name_len = 0;
                input_stream.read(reinterpret_cast<char *>(&name_len), sizeof(name_len));

                std::string filename(name_len, '\0');
                input_stream.read(&filename[0], name_len);

                uint64_t file_size = 0;
                input_stream.read(reinterpret_cast<char *>(&file_size), sizeof(file_size));

                std::string content(file_size, '\0');
                input_stream.read(&content[0], file_size);

                uint32_t stored_crc = 0;
                input_stream.read(reinterpret_cast<char *>(&stored_crc), sizeof(stored_crc));

                uint32_t calc_crc = CalculateCrc32(content.data(), file_size);

                if (stored_crc != calc_crc) {
                    std::cerr << "Checksum mismatch for: " << filename << '\n';
                    continue;
                }

                data_map[filename] = content;
            }

            return input_stream.good();
        }

        uint32_t SerializationUtils::CalculateCrc32(const char *data, size_t size) {
            uint32_t crc = crc32(0L, Z_NULL, 0);
            return crc32(crc, reinterpret_cast<const Bytef *>(data), size);
        }

    } // namespace utils
} // namespace rayshape
