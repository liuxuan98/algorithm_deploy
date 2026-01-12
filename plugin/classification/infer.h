#ifndef RAYSHAPE_INFER_H
#define RAYSHAPE_INFER_H

#ifndef ENABLE_3RD_OPENCV
#define ENABLE_3RD_OPENCV

#endif // !ENABLE_3RD_OPENCV

#include "dag/node.h"
#include "inference/inference.h"
/*单个分类模型的推理节点定义*/
namespace rayshape
{
    class ClassificationInfer: public dag::Node {
    public:
        ClassificationInfer(const std::string &name); /* node instance name*/
        /**节点名、链接的多个输入输出边*/

        ClassificationInfer(const std::string &name, std::vector<dag::Edge *> inputs,
                            std::vector<dag::Edge *> outputs);
        /** infer 还要考虑推理类型*/
        ClassificationInfer(const std::string &name, InferenceType type);
        ClassificationInfer(const std::string &name, std::vector<dag::Edge *> inputs,
                            std::vector<dag::Edge *> outputs, InferenceType type);

        virtual ~ClassificationInfer();

        virtual ErrorCode Init();
        virtual ErrorCode DeInit();
        // 设置推理类型 似乎用构造函数就可以
        virtual ErrorCode SetInferenceType(InferenceType inference_type);

        virtual ErrorCode Run() override; // 重载推理调度

    private:
        InferenceType type_ = InferenceType::NONE;

        std::shared_ptr<inference::Inference> inference_ = nullptr;

        std::set<std::string> inference_input_names_;
        std::set<std::string> inference_output_names_;
    };

    class ClassificationPostProcess: public dag::Node {
    public:
        ClassificationPostProcess(const std::string &name);

        ClassificationPostProcess(const std::string &name, std::vector<dag::Edge *> inputs,
                                  std::vector<dag::Edge *> outputs);

        virtual ~ClassificationPostProcess();

        virtual ErrorCode Run() override;
    };

} // namespace rayshape

#endif