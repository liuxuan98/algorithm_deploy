#ifndef _TYPE_UTILS_H_
#define _TYPE_UTILS_H_

#include "base/common.h"

namespace rayshape
{
    namespace utils
    {
        static unsigned int GetBytesSize(const DataType &data_type) {
            if (data_type == DATA_TYPE_FLOAT) {
                return 4;
            } else if (data_type == DATA_TYPE_HALF) {
                return 2;
            } else if (data_type == DATA_TYPE_INT8) {
                return 1;
            } else if (data_type == DATA_TYPE_UINT8) {
                return 1;
            } else if (data_type == DATA_TYPE_INT32) {
                return 4;
            } else if (data_type == DATA_TYPE_INT64) {
                return 8;
            } else if (data_type == DATA_TYPE_UINT32) {
                return 4;
            } else {
                // log
                return 0;
            }
        }
    } // namespace utils
} // namespace rayshape

#endif