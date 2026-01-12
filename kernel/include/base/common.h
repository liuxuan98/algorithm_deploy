#ifndef COMMON_H
#define COMMON_H

#include "glic_stl_include.h"
#include "macros.h"

namespace rayshape
{
    // remove namespace common
    // common enum type or data structure.
    enum class DataType {
        // none
        NONE = -2,
        // auto
        AUTO = -1,
        // float
        FLOAT = 0,
        // half float
        HALF = 1,
        // int8
        INT8 = 2,
        // uint8
        UINT8 = 3,
        // int32
        INT32 = 4,
        // int64
        INT64 = 5,
        // uint32
        UINT32 = 6
    };

    enum class DataFormat {
        // decided by device
        AUTO = -1,
        NC = 0,
        NCHW = 1,
        NHWC = 2,
        NHWC4 = 3,
        NCDHW = 4
    };

    enum class InferenceType { // inference/nn network type
        NONE = 0x0000,
        OPENVINO = 0x1000,
        TENSORRT = 0x2000,
        COREML = 0x3000,
        NCNN = 0x4000,
        MNN = 0x5000,
        ONNXRUNTIME = 0x6000
    };

    enum class ModelType { // prase model type
        NONE = 0x0000,
        OPENVINO = 0x1000,
        TENSORRT = 0x2000,
        COREML = 0x3000,
        NCNN = 0x4000,
        MNN = 0x5000,
        ONNX = 0x6000
    };

    enum class DeviceType {
        NONE = 0x0000,
        CPU = 0x0010,
        X86 = 0x0020,
        ARM = 0x0030,
        // Heterogeneous device
        OPENCL = 0x1010,
        CUDA = 0x1020,
        INTERL_NPU = 0x1030,
        INTERL_GPU = 0x1040
    };

    enum class MemoryType {
        NONE = -1,
        HOST = 0,  // cpu /x86 /arm host memory
        CUDA = 1,  // nvidia gpu memory
        OPENCL = 2 // opencl(intel_gpu/nvidia,arm,amd,qualcomm) gpu memory
    };

    enum class Precision { AUTO = -1, NORMAL = 0, HIGH = 1, LOW = 2 };

    typedef struct CustomRuntime {
        DeviceType device_type_ = DeviceType::NONE;
        InferenceType inference_type_ = InferenceType::NONE;
        ModelType model_type_ = ModelType::NONE;
        int num_thread_ = -1;
        bool use_gpu_ = false;
    } CustomRuntime;

    using DimsVector = std::vector<int>;
    using IntVector = std::vector<int>;
    using SizeVector = std::vector<size_t>;

    typedef enum ParallelType {
        PARALLEL_TYPE_NONE = 0x0001,
        PARALLEL_TYPE_SEQUENTIAL = 0x0001 << 1,
        PARALLEL_TYPE_TASK = 0x0001 << 2,
        PARALLEL_TYPE_PIPELINE = 0x0001 << 3
    } ParallelType;

    typedef enum EdgeType {
        EDGE_TYPE_FIXED = 0x0001,
        EDGE_TYPE_PIPELINE = 0x0001 << 1,
        EDGE_TYPE_NONE = 0x0001 << 2
    } EdgeType;

    typedef enum NodeColorType {
        NODE_COLOR_WHITE = 0, // not visited
        NODE_COLOR_GRAY = 1,  // visiting
        NODE_COLOR_BLACK = 2  // visited
    } NodeColorType;

    enum class EdgeUpdateFlag { Complete = 0x0001, Terminate = 0x0002, Error = 0x0003 };

    class RS_PUBLIC NonCopyable {
    public:
        NonCopyable() = default;

        NonCopyable(const NonCopyable &) = delete;
        NonCopyable &operator=(const NonCopyable &) = delete;

        NonCopyable(NonCopyable &&) = delete;
        NonCopyable &operator=(NonCopyable &&) = delete;
    };

} // namespace rayshape

#endif // COMMON_H
