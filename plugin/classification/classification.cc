#include "classification.h"

namespace rayshape
{
    // 前处理
    /* 完成简单图的构建，并完成节点的创建
     */
    ClassificationGraph::ClassificationGraph(const std::string &name) : Graph(name) {
        // 以命名方式构建的图节点,在构造函数中创建输入输出边

        RS_LOGI("begin default construct ClassificationGraph\n");

        dag::Edge *input_edge = this->CreateEdge("input_edge");
        dag::Edge *output_edge = this->CreateEdge("output_edge");
        if (input_edge == nullptr || output_edge == nullptr) {
            RS_LOGE("create input or output edge failed\n");
        }

        RS_LOGI("finish default construct ClassificationGraph\n");
    }

    ClassificationGraph::ClassificationGraph(const std::string &name,
                                             std::vector<dag::Edge *> inputs,
                                             std::vector<dag::Edge *> outputs) :
        Graph(name, inputs, outputs) {
        RS_LOGI("begin construct ClassificationGraph\n");
        // 直接调用父类传递输入输出边进行传递.

        RS_LOGI("finish construct ClassificationGraph\n");
    }

    ClassificationGraph::~ClassificationGraph() {
        // node 归属管理图内部构造和管理
    }
    // todo make graph.
    ErrorCode ClassificationGraph::MakeGraph(InferenceType inference_type) {
        //内部直接设定并行方式
        //SetParall

        // 边都是节点自己管理
        std::vector<dag::Edge *> graph_all_input = this->GetAllInput();

        dag::Edge *graph_input = this->GetInputEdge(0);
        //dag::Edge *preprocess_output = new dag::Edge("preprocess_output_edge"); // TODO
        dag::Edge *preprocess_output =  this->CreateEdge("preprocess_output_edge");
        //先采用手动构图的方式进行
        // new
        //  Create preprocessing node.
        pre_ = dynamic_cast<PreProcess *>(this->CreateNode<PreProcess>(
            "classification_preprocess", graph_input,
            preprocess_output)); // pre等节点生命周期会比和图的生命周期一致
        if (pre_ == nullptr) {
            RS_LOGE("Failed to create preprocessing node.\n");
            is_constructed_ = false;
            return RS_NODE_STATU_ERROR;
        }
        
        dag::Edge *infer_output = this->CreateEdge("infer_output_edge");
        //  Create inference node for classification.
        infer_ = dynamic_cast<ClassificationInfer *>(this->CreateNode<ClassificationInfer>(
            "classification_infer", preprocess_output, infer_output));
        if (infer_ == nullptr) {
            RS_LOGE("Failed to create inference node\n");
            is_constructed_ = false;
            return RS_NODE_STATU_ERROR;
        }

        ErrorCode ret = infer_->SetInferenceType(inference_type);
        if (ret != RS_SUCCESS) {
            RS_LOGE("infer_ SetInferenceType failed!\n");
            return ret;
        }
        //简单图构建方式
        //dag::Edge *postprocess_output = this->CreateEdge("postprocess_output_edge"); noneed
        dag::Edge *graph_output_edge =
            this->GetEdge("output_edge"); //按照edge_name 来从edge_repository_来obtain/acquire edge.问题是外部构建的
        dag::Edge *graph_output = this->GetOutputEdge(0);
        // Create postprocessing node.
        post_ = dynamic_cast<ClassificationPostProcess *>(this->CreateNode<ClassificationPostProcess>(
                "classification_postprocess", infer_output, graph_output));
        if (post_ == nullptr) {
            RS_LOGE("Failed to create postprocessing node");
            is_constructed_ = false;
            return RS_NODE_STATU_ERROR;
        }

        return RS_SUCCESS;
    }

    std::vector<dag::Edge *> ClassificationGraph::Forward(std::vector<dag::Edge *> inputs) {
        // 动态组图时必须加载
        std::vector<dag::Edge *> preprocess_output = (*pre_)(inputs);
        std::vector<dag::Edge *> infer_output = (*infer_)(preprocess_output);
        std::vector<dag::Edge *> post_output = (*post_)(infer_output);
        // ## TODO

        return post_output;
    }

} // namespace rayshape