#ifndef _COMMON_H_
#define _COMMON_H_

#include "glic_stl_include.h"
#include "macros.h"

namespace rayshape
{
    // remove namespace common
    // common enum type or data structure.
    typedef enum {
        // none
        DATA_TYPE_NONE = -2,
        // auto
        DATA_TYPE_AUTO = -1,
        // float
        DATA_TYPE_FLOAT = 0,
        // half float
        DATA_TYPE_HALF = 1,
        // int8
        DATA_TYPE_INT8 = 2,
        // uint8
        DATA_TYPE_UINT8 = 3,
        // int32
        DATA_TYPE_INT32 = 4,
        // int64
        DATA_TYPE_INT64 = 5,
        // uint32
        DATA_TYPE_UINT32 = 6

    } DataType;

    typedef enum {
        // decided by device
        DATA_FORMAT_AUTO = -1,
        DATA_FORMAT_NC = 0,
        DATA_FORMAT_NCHW = 1,
        DATA_FORMAT_NHWC = 2,
        DATA_FORMAT_NHWC4 = 3,
        DATA_FORMAT_NCDHW = 4
    } DataFormat;

    typedef enum { // inference/nn network type
        INFERENCE_TYPE_NONE = 0x0000,
        INFERENCE_TYPE_OPENVINO = 0x1000,
        INFERENCE_TYPE_TENSORRT = 0x2000,
        INFERENCE_TYPE_COREML = 0x3000,
        INFERENCE_TYPE_NCNN = 0x4000

    } InferenceType;

    typedef enum { // prase model type
        MODEL_TYPE_NONE = 0x0000,
        MODEL_TYPE_OPENVINO = 0x1000,
        MODEL_TYPE_TENSORRT = 0x2000,
        MODEL_TYPE_COREML = 0x3000,
        MODEL_TYPE_NCNN = 0x4000
    } ModelType;

    typedef enum {
        DEVICE_TYPE_NONE = 0x0000,
        DEVICE_TYPE_CPU = 0x0010,
        DEVICE_TYPE_X86 = 0x0020,
        DEVICE_TYPE_ARM = 0x0030,
        // Heterogeneous device
        DEVICE_TYPE_OPENCL = 0x1010,
        DEVICE_TYPE_CUDA = 0x1020,
        DEVICE_TYPE_INTERL_NPU = 0x1030,
        DEVICE_TYPE_INTERL_GPU = 0x1040

    } DeviceType;

    typedef enum {
        MEM_TYPE_NONE = -1,
        MEM_TYPE_HOST = 0,  // cpu /x86 /arm host memory
        MEM_TYPE_CUDA = 1,  // nvidia gpu memory
        MEM_TYPE_OPENCL = 2 // opencl(intel_gpu/nvidia,arm,amd,qualcomm) gpu memory
    } MemoryType;

    typedef enum {
        PRECISION_AUTO = -1,
        PRECISION_NORMAL = 0,
        PRECISION_HIGH = 1,
        PRECISION_LOW = 2
    } Precision;

    typedef struct CustomRuntime {
        DeviceType device_type = DEVICE_TYPE_NONE;
        InferenceType inference_type = INFERENCE_TYPE_NONE;
        ModelType model_type = MODEL_TYPE_NONE;
        int num_thread = -1;
        bool use_gpu = false;
    } CustomRuntime;

    using DimsVector = std::vector<int>;
    using IntVector = std::vector<int>;
    using SizeVector = std::vector<size_t>;

} // namespace rayshape

#endif