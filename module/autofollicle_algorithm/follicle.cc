#include "follicle.h"

void MatToBlob()
{

} // 建议写在公共的函数部分

namespace rayshape
{
    namespace follicle
    {
        Follicle::Follicle()
        {
        }

        Follicle::~Follicle()
        {
        }

        ErrorCode Follicle::Init(const char *model_json)
        {
            // json 解析
            ErrorCode ret = RS_SUCCESS;

            // 解析完成,获得模型路径和cus_runtime
            std::string model_path = model_json["model_path"];
            InferenceType = model_json["inference_type"];
            CustomRuntime runtime_param = model_json["custom_runtime"]; // 定义运行时参数

            Model *model_hanle = new Model(model_path); // 建议创建unique_ptr
            // m_inference_.reset(model_hanle);
            m_inference_ = createInference(InferenceType); // 创建推理对象,考虑私有成员持有，有需要可考虑
                                                           // 特有函数可考虑动态转换
            ret = Follicle->Init(model_hanle, runtime_param);
            if (ret != RS_SUCCESS)
            {
                // LOG_ERROR("Init inference failed!");
                return ret;
            }

            return ret;
        }

        ErrorCode Follicle::Process()
        {
        }

    }
}