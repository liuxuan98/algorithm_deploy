#ifndef _ERROR_H_
#define _ERROR_H_

#include "glic_stl_include.h"

namespace rayshape
{

    enum ErrorCode
    {
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

        // device errcode
        RS_DEVICE_NOT_SUPPORT = 0x4000,
        RS_DEVICE_INVALID = 0x4001,

        // cuda errcode
        RS_CUDA_TENSORRT_ERROR = 0x5000,
        RS_CUDA_MEMCPY_ERROR = 0x5001
    };

}

#endif