#include "cardiac_plugin/include/sp_Infer.h"
#include "dag/graph.h" //使用graph

namespace rayshape
{
    SpClsInferNode::SpClsInferNode(const std::string &name, std::vector<dag::Edge *> inputs,
                                   std::vector<dag::Edge *> outputs) : Node(name, inputs, outputs) {
        // 算法运行参数.
    }
    // single input ,
    dag::Graph *CreatSerialGraph(const std::string &name, std::vector<dag::Edge *> input,
                                 std::vector<dag::Edge *> output) {
        dag::Graph *graph = new dag::Graph(name, {input}, {output});
    }

} // namespace rayshape
