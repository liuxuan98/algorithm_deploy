#include "node.h"
// 切面识别模块

namespace rayshape
{
    // 创建一个serial graph将三个节点串联.
    dag::Graph *CreatSerialGraph(const std::string &name, std::vector<dag::Edge *> input,
                                 std::vector<dag::Edge *> output);

    // 将preprocess、infer、postprocess统一放在一个节点中,因为本身也是依赖执行(串行执行)的.
    class SpClsInferNode: dag::Node {
    public:
        SpClsInferNode(const std::string &name, std::vector<dag::Edge *> inputs,
                       std::vector<dag::Edge *> outputs);

        virtual ~SpClsInferNode();

        virtual ErrorCode Run();

    private:
    };

    class PreProcessNode: public dag::Node {
    public:
        PreProcessNode(const std::string &name, std::vector<dag::Edge *> inputs,
                       std::vector<dag::Edge *> outputs);

        virtual ~PreProcessNode();

        virtual ErrorCode Run();

    private:
    };

    class PostProcessNode: public dag::Node {
    public:
        PostProcessNode(const std::string &name, std::vector<dag::Edge *> inputs,
                        std::vector<dag::Edge *> outputs);

        virtual ~PostProcessNode();

        virtual ErrorCode Run();

    private:
    };

} // namespace rayshape