#ifndef TYPE_UTILS_H
#define TYPE_UTILS_H

#include "base/common.h"

namespace rayshape
{
    namespace utils
    {
        // Size constants for different data types in bytes
        static constexpr unsigned int FLOAT_SIZE = 4;
        static constexpr unsigned int HALF_SIZE = 2;
        static constexpr unsigned int INT8_SIZE = 1;
        static constexpr unsigned int UINT8_SIZE = 1;
        static constexpr unsigned int INT32_SIZE = 4;
        static constexpr unsigned int INT64_SIZE = 8;
        static constexpr unsigned int UINT32_SIZE = 4;

        static unsigned int GetBytesSize(const DataType &data_type) {
            if (data_type == DataType::FLOAT) {
                return FLOAT_SIZE;
            } else if (data_type == DataType::HALF) {
                return HALF_SIZE;
            } else if (data_type == DataType::INT8) {
                return INT8_SIZE;
            } else if (data_type == DataType::UINT8) {
                return UINT8_SIZE;
            } else if (data_type == DataType::INT32) {
                return INT32_SIZE;
            } else if (data_type == DataType::INT64) {
                return INT64_SIZE;
            } else if (data_type == DataType::UINT32) {
                return UINT32_SIZE;
            } else {
                // log
                return 0;
            }
        }
    } // namespace utils
} // namespace rayshape

#endif // TYPE_UTILS_H
