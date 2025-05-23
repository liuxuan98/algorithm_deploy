#ifndef _INFERENCE_PARAM_H_
#define _INFERENCE_PARAM_H_

#include "base/common.h"
#include "base/macros.h"

// todo 不考虑使用该类,太繁琐
namespace rayshape
{

    // 推理类持有InferenceParam ，通用的配置，各推理框架有些不同的配置，通过继承可以实现

    class RS_PUBLIC InferenceParam
    {
    public:
        // infer

        // 获得函数

    private:
        InferenceType infer_type_;      // 推理类型
        ModelType model_type_;          // 模型类型
        DeviceType device_type_;        // 模型推理的设备类型
        PrecisionType precision_type_;  // 模型推理的精度类型
        bool is_dynamic_shape_ = false; // 是否动态shape
        ShapeMap min_shape_;            // 当为动态时最小shape
        ShapeMap opt_shape_;            // 当为动态时最优shape
        ShapeMap min_shape_;            // 当为动态时最大shape

        int input_num_ = 0;                         // 输入的数量
        std::vector<std::string> input_name_;       // 输入的名称
        std::vector<std::vector<int>> input_shape_; // 输入的形状
        int output_num_ = 0;                        // 输出的数量
        std::vector<std::string> output_name_;      // 输出的名称

        std::vector<std::string> library_path_ = {} // 第三方推理框架的动态路径

        int num_thread_ = 1; // CPU推理的线程数
    };

} // namespace rayshape

#endif
