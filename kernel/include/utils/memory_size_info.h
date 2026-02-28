// 内存计算function 工具

#ifndef MEMORY_SIZE_INFO_H
#define MEMORY_SIZE_INFO_H

#include "type_utils.h"
#include "memory_manager/blob.h"

namespace rayshape
{
    namespace utils
    {
        static size_t CalculateDims(const Dims &dims) {
            size_t result = 1;
            if (dims.size == 0) {
                return 0;
            }
            for (int i = 0; i < dims.size; i++) {
                result *= dims.value[i];
            }
            return result;
        }

        static size_t CalculateMemorySize(const Dims &dims, const DataType &data_type) {
            size_t dims_size = CalculateDims(dims);

            int type_size = GetBytesSize(data_type); // 为0的情况

            size_t byte_size = dims_size * type_size;

            return byte_size;
        }

    } // namespace utils
} // namespace rayshape

#endif // MEMORY_SIZE_INFO_H
