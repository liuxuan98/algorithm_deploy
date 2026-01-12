#ifndef ERROR_H
#define ERROR_H

#include "glic_stl_include.h"
#include "logger.h"

namespace rayshape
{
    enum ErrorCode {
        RS_SUCCESS = 0,

        // param errcode
        RS_INVALID_PARAM = 0x1000,
        RS_INVALID_PARAM_VALUE = 0x1001,
        RS_INVALID_PARAM_NAME = 0x1002,
        RS_INVALID_PARAM_FORMAT = 0x1003,
        RS_NULL_PARAM = 0x1004,

        // model errcode
        RS_INVALID_MODEL = 0x2000,
        RS_MODEL_COMPILE_ERROR = 0x2001,
        RS_MODEL_ERROR = 0x2002,

        // common errcode
        RS_INVALID_LICENSE = 0x3000,
        RS_INVALID_FILE = 0x3001,
        RS_OUTOFMEMORY = 0x3002,
        RS_NOT_IMPLEMENT = 0x3003,
        RS_INVALID_JSON = 0x3004,

        // device errcode
        RS_DEVICE_NOT_SUPPORT = 0x4000,
        RS_DEVICE_INVALID = 0x4001,

        // cuda errcode
        RS_CUDA_TENSORRT_ERROR = 0x5000,
        RS_CUDA_MEMCPY_ERROR = 0x5001,
        RS_CUDA_MEMSET_ERROR = 0x5002,
        RS_CUDA_MALLOC_ERROR = 0x5003,
        RS_CUDA_FREE_ERROR = 0x5004,

        // file errcode
        RS_FILE_OPEN_ERROR = 0x6000,
        RS_FILE_READ_ERROR = 0x6001,
        RS_FILE_WRITE_ERROR = 0x6002,
        RS_FILE_CLOSE_ERROR = 0x6003,

        // RS_CUDA_MEMCPY_ERROR = 0x5001,

        // dag errcode
        RS_NODE_STATU_ERROR = 0x6100,
        RS_DAG_STATU_ERROR = 0x6101,
        // thread pool error
        RS_THREAD_POLL_ERROR = 0x6200,

        RS_UNKNOWN
    };

#define RS_CHECK_PARAM_NULL_RET_STATUS(param, str)                                                 \
    do {                                                                                           \
        if (!param) {                                                                              \
            RS_LOGE("%s\n", str);                                                                  \
            return RS_INVALID_PARAM_VALUE;                                                         \
        }                                                                                          \
    } while (0)

#define RS_RETURN_ON_NEQ(status, expected, str)                                                    \
    do {                                                                                           \
        if (status != (expected)) {                                                                \
            RS_LOGE("%s\n", str);                                                                  \
            return status;                                                                         \
        }                                                                                          \
    } while (0)

} // namespace rayshape

#endif // ERROR_H
