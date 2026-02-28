#ifndef SERIALIZATION_UTILS_H
#define SERIALIZATION_UTILS_H

#include "base/glic_stl_include.h"
#include <zlib.h>

namespace rayshape
{
    namespace utils
    {

        /**
         * @brief Serialization utility class for packing and unpacking files
         * @details Provides simple binary serialization with CRC32 checksums
         */
        class SerializationUtils {
        public:
            /**
             * @brief Serialize multiple files into a single binary file
             * @param input_files Vector of input file paths to pack
             * @param output_file Output packed file path
             * @return true if successful, false otherwise
             */
            static bool SerializeFiles(const std::vector<std::string> &input_files,
                                       const std::string &output_file);

            /**
             * @brief Deserialize a packed binary file into multiple files
             * @param input_file Input packed file path
             * @param output_dir Output directory for unpacked files
             * @return true if successful, false otherwise
             */
            static bool DeSerializeFiles(const std::string &input_file,
                                         const std::string &output_dir);

            /**
             * @brief Serialize data directly to a binary stream
             * @param data_map Map of filename to data content
             * @param output_stream Output binary stream
             * @return true if successful, false otherwise
             */
            static bool SerializeToStream(const std::map<std::string, std::string> &data_map,
                                          std::ostream &output_stream);

            /**
             * @brief Deserialize data from a binary stream
             * @param input_stream Input binary stream
             * @param data_map Output map of filename to data content
             * @return true if successful, false otherwise
             */
            static bool DeserializeFromStream(std::istream &input_stream,
                                              std::map<std::string, std::string> &data_map);

        private:
            static const char MAGIC_HEADER[4];

            /**
             * @brief Calculate CRC32 checksum for data
             * @param data Data buffer
             * @param size Data size
             * @return CRC32 checksum
             */
            static uint32_t CalculateCrc32(const char *data, size_t size);

            /**
             * @brief Extract filename from full path (C++11 compatible)
             * @param file_path Full file path
             * @return Filename only
             */
            static std::string ExtractFilename(const std::string &file_path);

            /**
             * @brief Join path components (C++11 compatible)
             * @param dir Directory path
             * @param filename Filename
             * @return Combined path
             */
            static std::string JoinPath(const std::string &dir, const std::string &filename);

            /**
             * @brief Create directory recursively (C++11 compatible)
             * @param dir_path Directory path to create
             * @return true if successful, false otherwise
             */
            static bool CreateDirectories(const std::string &dir_path);
        };

    } // namespace utils
} // namespace rayshape

#endif // SERIALIZATION_UTILS_H
