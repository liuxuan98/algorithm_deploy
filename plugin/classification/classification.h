#ifndef RAYSHAPE_CLASSIFICATION_H
#define RAYSHAPE_CLASSIFICATION_H

#include "dag/graph.h"
#include "infer.h"
#include "preprocess.h"

// #define ENABLE_3RD_OPENCV

namespace rayshape
{
    // 需要手动构图时提供边信息
    class RS_PUBLIC ClassificationGraph: public dag::Graph {
    public:
        ClassificationGraph(const std::string &name);

        ClassificationGraph(
            const std::string &name, std::vector<dag::Edge *> inputs,
            std::vector<dag::Edge *> outputs); // LNK2019 C++模板函数定义未实例化的链接问题，

        virtual ~ClassificationGraph();

        ErrorCode MakeGraph(
            InferenceType inference_type); // 初始化构建图的方法,提供图的节点创建信息并提供边

        std::vector<dag::Edge *> Forward(
            std::vector<dag::Edge *> inputs); // 自由的组装图的处理流程通过forward()函数实现

    private:
        PreProcess *pre_ = nullptr;                 // preprocess node pointer
        ClassificationInfer *infer_ = nullptr;      // inference node pointer
        ClassificationPostProcess *post_ = nullptr; // postprocess node pointer
    };

} // namespace rayshape

#endif
