#include "dag/graph.h"
#include "dag/engine/parallel_task_engine.h"
#include "dag/engine/sequential_engine.h"
#include "dag/engine/parallel_pipeline_engine.h"

namespace rayshape
{
    namespace dag
    {
        Graph::Graph(const std::string &name) : Node(name) {
            is_constructed_ = true;
        }

        Graph::Graph(const std::string &name, std::vector<Edge *> inputs,
                     std::vector<Edge *> outputs) : Node(name, inputs, outputs) {
            is_constructed_ = true;

            for (auto input : inputs) {
                if (AddEdge(input) == nullptr) {
                    is_constructed_ = false;
                    return;
                }
            }

            for (auto output : outputs) {
                if (AddEdge(output) == nullptr) {
                    is_constructed_ = false;
                    return;
                }
            }

            is_constructed_ = true;
        }

        Graph::~Graph() {
            if (this->GetInitStatus()) {
                this->Deinit();
                this->SetInitStatus(false);
            }
            for (auto node_wrapper : node_repository_) {
                if (!node_wrapper->is_external_) {
                    delete node_wrapper->node_;
                    node_wrapper->node_ = nullptr;
                }
                delete node_wrapper;
            }
            for (auto edge_wrapper : edge_repository_) {
                if (!edge_wrapper->is_external_) {
                    delete edge_wrapper->edge_;
                    edge_wrapper->edge_ = nullptr;
                }

                delete edge_wrapper;
            }
            node_repository_.clear();
            used_node_names_.clear();
            edge_repository_.clear();
            used_edge_names_.clear();
        }

        ErrorCode Graph::SetInputs(std::vector<Edge *> inputs) {
            ErrorCode ret = Node::SetInputs(inputs);

            for (auto input : inputs) {
                auto edge_wrapper = this->AddEdge(input, true);
                if (edge_wrapper == nullptr) {
                    RS_LOGE("AddEdge for input[%s] failed!\n", input->GetName().c_str());
                    return RS_DAG_STATU_ERROR;
                }
            }

            return ret;
        }

        ErrorCode Graph::SetOutputs(std::vector<Edge *> outputs) {
            ErrorCode ret = Node::SetOutputs(outputs);

            for (auto output : outputs) {
                auto edge_wrapper = this->AddEdge(output, true);
                if (edge_wrapper == nullptr) {
                    RS_LOGE("AddEdge for output[%s] failed!\n", output->GetName().c_str());
                    return RS_DAG_STATU_ERROR;
                }
            }

            return ret;
        }

        /**
         * start node manager API
         */

        ErrorCode Graph::AddNode(Node *node, bool is_external) {
            if (node == nullptr) {
                RS_LOGE("node is null!");
                return RS_INVALID_PARAM;
            }

            if (this == node) {
                RS_LOGE("Graph[%s] cannot add itself as node\n", this->GetName().c_str());
                return RS_INVALID_PARAM_VALUE;
            }

            if (used_node_names_.find(node->GetName()) != used_node_names_.end()) {
                RS_LOGW("Warning: node name[%s] is already used!\n", node->GetName().c_str());
                return RS_INVALID_PARAM_VALUE;
            }
            ErrorCode ret = RS_SUCCESS;

            NodeWrapper *node_wrapper = new NodeWrapper();
            node_wrapper->is_external_ = is_external;
            node_wrapper->node_ = node;
            node_wrapper->name_ = node->GetName();
            for (auto input : node->GetAllInput()) {
                EdgeWrapper *input_wrapper = FindEdgeWrapper(edge_repository_, input);
                if (input_wrapper == nullptr) {
                    input_wrapper = this->AddEdge(input, true);
                }

                InsertUnique(input_wrapper->consumers_, node_wrapper);
            }
            for (auto output : node->GetAllOutput()) {
                EdgeWrapper *output_wrapper = FindEdgeWrapper(edge_repository_, output);
                if (output_wrapper == nullptr) {
                    output_wrapper = this->AddEdge(output, true);
                }
                //
                InsertUnique(output_wrapper->producers_, node_wrapper);
            }

            node_repository_.emplace_back(node_wrapper); // new node wrapper add into node
                                                         // repository
            used_node_names_.insert(node->GetName());    // add node name into used node names

            node->SetGraph(this); // node know which graph it belongs to

            return ret;
        }

        Node *Graph::GetNode(const std::string &name) {
            for (auto node_wrapper : node_repository_) {
                if (node_wrapper->name_ == name) {
                    return node_wrapper->node_;
                }
            }

            return nullptr;
        }

        Node *Graph::GetNode(int index) {
            if (index < 0 || index >= node_repository_.size()) {
                return nullptr;
            }
            return node_repository_[index]->node_;
        }

        int Graph::GetNodeCount() const {
            return node_repository_.size();
        }

        /**
         * end node manager API
         */

        /**
         * start edge manager API
         */
        Edge *Graph::CreateEdge(const std::string &name) {
            std::string edge_name = name;
            if (edge_name.empty()) {
                edge_name = "edge_" + GetUniqueString();
            }
            if (used_edge_names_.find(edge_name) != used_edge_names_.end()) {
                RS_LOGE("edge name %s already exists\n", edge_name.c_str());
                return nullptr;
            }
            Edge *edge = new Edge(edge_name);
            EdgeWrapper *edge_wrapper = new EdgeWrapper();
            edge_wrapper->is_external_ = false;
            edge_wrapper->edge_ = edge;
            edge_wrapper->name_ = edge_name;
            edge_repository_.emplace_back(edge_wrapper);
            used_edge_names_.insert(edge_name);

            return edge;
        }

        EdgeWrapper *Graph::AddEdge(Edge *edge, bool is_external) {
            if (edge == nullptr) {
                RS_LOGE("edge is null\n");
                return nullptr;
            }
            if (used_edge_names_.find(edge->GetName()) != used_edge_names_.end()) {
                for (auto edge_wrapper : edge_repository_) {
                    if (edge_wrapper->edge_ == edge) {
                        return edge_wrapper;
                    }
                }
            }
            EdgeWrapper *edge_wrapper = new EdgeWrapper();
            edge_wrapper->is_external_ = is_external;
            edge_wrapper->edge_ = edge;
            edge_wrapper->name_ = edge->GetName();
            edge_repository_.emplace_back(edge_wrapper);
            used_edge_names_.insert(edge->GetName());
            return edge_wrapper;
        }

        Edge *Graph::GetEdge(const std::string &name) {
            for (EdgeWrapper *edge_wrapper : edge_repository_) {
                if (edge_wrapper->name_ == name) {
                    return edge_wrapper->edge_;
                }
            }
            return nullptr;
        }

        ErrorCode Graph::UpdateEdge(EdgeWrapper *edge_wrapper, Edge *edge, bool is_external) {
            if (edge == nullptr) {
                RS_LOGE("edge is null\n");
                return RS_INVALID_PARAM;
            }

            //  #TODO.
            return RS_SUCCESS;
        }

        /**
         *end edge manager API
         */

        /**
         *更新节点的输入输出 适用于图(节点)的输入输出更新
         */
        ErrorCode Graph::UpdateNodeIO(Node *node, std::vector<Edge *> inputs,
                                      std::vector<Edge *> outputs) {
            ErrorCode ret = RS_SUCCESS;
            // find node_wrapper from node(wrapper) repository
            NodeWrapper *node_wrapper = nullptr;
            for (auto wrapper : node_repository_) {
                if (wrapper->node_ == node) {
                    node_wrapper = wrapper;
                    break;
                }
            }

            if (node_wrapper == nullptr) {
                RS_LOGE("can't find node_wrapper!");
                return RS_INVALID_PARAM_VALUE;
            }

            for (auto input : inputs) {
                EdgeWrapper *edge_wrapper = FindEdgeWrapper(edge_repository_, input->GetName());
                if (edge_wrapper == nullptr) {
                    edge_wrapper = this->AddEdge(input, true);
                    if (edge_wrapper == nullptr) {
                        RS_LOGE("addEdge failed!");
                        return RS_INVALID_PARAM_VALUE;
                    }
                } else {
                    if (edge_wrapper->edge_ != input) {
                        RS_LOGI("node[%s] updateEdge: %s\n", node->GetName().c_str(),
                                input->GetName().c_str());
                        UpdateEdge(edge_wrapper, input, true);
                    }
                }
                // add consumer
                InsertUnique(edge_wrapper->consumers_, node_wrapper);
            }

            for (auto output : outputs) {
                EdgeWrapper *edge_wrapper = FindEdgeWrapper(edge_repository_, output->GetName());
                if (edge_wrapper == nullptr) {
                    edge_wrapper = this->AddEdge(output, true);
                    if (edge_wrapper == nullptr) {
                        RS_LOGE("addEdge failed!");
                        return RS_INVALID_PARAM_VALUE;
                    }
                } else {
                    if (edge_wrapper->edge_ != output) {
                        RS_LOGI("node[%s] updateEdge: %s\n", node->GetName().c_str(),
                                output->GetName().c_str());
                        UpdateEdge(edge_wrapper, output, true);
                    }
                }
                InsertUnique(edge_wrapper->producers_, node_wrapper);
            }

            return ret;
        }

        ErrorCode Graph::MarkInputEdge(std::vector<Edge *> inputs) {
            for (auto input : inputs) {
                InsertUnique(inputs_, input);
            }
            return RS_SUCCESS;
        }

        ErrorCode Graph::MarkOutputEdge(std::vector<Edge *> outputs) {
            for (auto output : outputs) {
                InsertUnique(outputs_, output);
            }
            return RS_SUCCESS;
        }

        /**
         * 图的初始化构建和运行.
         *
         * @return
         */
        ErrorCode Graph::Init() {
            ErrorCode ret = RS_SUCCESS;

            ret = this->Construct();
            if (ret != RS_SUCCESS) {
                RS_LOGE("graph Construct failed!");
                return ret;
            }

            ret = this->InitExecuteEngine();
            if (ret != RS_SUCCESS) {
                RS_LOGE("graph InitExecuteEngine failed!");
                return ret;
            }

            // set init flag. 设置初始化标志
            SetInitStatus(true);

            return ret;
        }

        ErrorCode Graph::Deinit() {
            ErrorCode ret = RS_SUCCESS;

            if (execute_engine_ != nullptr) {
                ret = execute_engine_->DeInit();
                RS_RETURN_ON_NEQ(ret, RS_SUCCESS, "execute_engine_ deinit failed!");
            } else {
                for (auto node_wrapper : node_repository_) {
                    if (node_wrapper->node_->GetInitStatus()) {
                        ret = node_wrapper->node_->Deinit();
                        RS_RETURN_ON_NEQ(ret, RS_SUCCESS, "node deinit failed!");
                        node_wrapper->node_->SetInitStatus(false);
                    }
                }
            }
            // set graph init flag false.
            SetInitStatus(false);

            return ret;
        }

        ErrorCode Graph::Construct() {
            ErrorCode ret = RS_SUCCESS;

            for (auto node_wrapper : node_repository_) {
                // add consumption/Producer to node,construt node relation(predecessors,successors).
                RS_CHECK_PARAM_NULL_RET_STATUS(node_wrapper->node_, "node_wrapper node is null!");
                std::vector<Edge *> inputs = node_wrapper->node_->GetAllInput();
                std::vector<Edge *> outputs = node_wrapper->node_->GetAllOutput();
                this->AddNodeInputAndOutput(node_wrapper, inputs, outputs);
            }

            for (auto edge_wrapper : edge_repository_) {
                RS_CHECK_PARAM_NULL_RET_STATUS(edge_wrapper->edge_,
                                               "edge_repository_ edge is null!");
            }

            for (auto node_wrapper : node_repository_) {
                Node *node = node_wrapper->node_;

                node->SetParallelType(parallel_type_);
                std::vector<Edge *> inputs = node->GetAllInput();
                for (auto input : inputs) {
                    EdgeWrapper *in_edge_wrapper = FindEdgeWrapper(edge_repository_, input);
                    RS_CHECK_PARAM_NULL_RET_STATUS(in_edge_wrapper, "in_edge_wrapper is null!");
                    for (auto producer : in_edge_wrapper->producers_) {
                        InsertUnique(node_wrapper->predecessors_, producer);
                    }
                }

                std::vector<Edge *> outputs = node->GetAllOutput();

                for (auto output : outputs) {
                    EdgeWrapper *out_edge_wrapper = FindEdgeWrapper(edge_repository_, output);
                    RS_CHECK_PARAM_NULL_RET_STATUS(out_edge_wrapper, "out_edge_wrapper is null!");
                    for (auto consumer : out_edge_wrapper->consumers_) {
                        InsertUnique(node_wrapper->successors_, consumer);
                    }
                }
            }

            for (auto edge_wrapper : edge_repository_) {
                std::vector<Node *> producers;
                for (auto producer : edge_wrapper->producers_) {
                    producers.emplace_back(producer->node_);
                }

                std::vector<Node *> consumers;
                for (auto consumer : edge_wrapper->consumers_) {
                    consumers.emplace_back(consumer->node_);
                }

                ErrorCode ret = edge_wrapper->edge_->SetParallelType(parallel_type_);
                RS_RETURN_ON_NEQ(ret, RS_SUCCESS, "setParallelType failed!");

                ret = edge_wrapper->edge_->IncreaseProducers(producers);
                RS_RETURN_ON_NEQ(ret, RS_SUCCESS, "edge IncreaseProducers failed!");

                ret = edge_wrapper->edge_->IncreaseConsumers(consumers);
                RS_RETURN_ON_NEQ(ret, RS_SUCCESS, "edge IncreaseConsumers failed!");

                ret = edge_wrapper->edge_->Construct();
                RS_RETURN_ON_NEQ(ret, RS_SUCCESS, "edge construct failed!");

                // pipeline use
                // ret = edge_wrapper->edge_->SetQueueMaxSize(queue_max_size_)
                // RS_RETURN_ON_NEQ(ret, RS_SUCCESS, "edge setQueueMaxSize failed!");
            }

            // #TODO:引入cuda流 对于节点的运行

            for (auto edge_wrapper : edge_repository_) {
                if (edge_wrapper->producers_.empty()) {
                    auto it = std::find(inputs_.begin(), inputs_.end(), edge_wrapper->edge_);
                    if (it == inputs_.end()) {
                        inputs_.emplace_back(edge_wrapper->edge_);
                    }
                }
            }

            for (auto edge_wrapper : edge_repository_) {
                if (edge_wrapper->consumers_.empty()) {
                    auto it = std::find(outputs_.begin(), outputs_.end(), edge_wrapper->edge_);
                    if (it == outputs_.end()) {
                        outputs_.emplace_back(edge_wrapper->edge_);
                    }
                }
            }

            // 如果不是图中的内部节点,(该图不是其他图的嵌入节点)
            if (!is_inner_) {
                for (auto iter : outputs_) {
                    // iter->MarkGraphOutput();  #todo.
                }
            }

            return ret;
        }

        ErrorCode Graph::InitExecuteEngine() {
            ErrorCode ret = RS_SUCCESS;
            // creat execute engine
            if (parallel_type_ == ParallelType::PARALLEL_TYPE_NONE) {
                execute_engine_ = std::make_shared<SequentialEngine>();
            } else if (parallel_type_ == ParallelType::PARALLEL_TYPE_SEQUENTIAL) {
            } else if (parallel_type_ == ParallelType::PARALLEL_TYPE_TASK) {
                execute_engine_ = std::make_shared<ParallelTaskEngine>();
            } else if (parallel_type_ == ParallelType::PARALLEL_TYPE_PIPELINE) {
                execute_engine_ = std::make_shared<ParallelPipelineEngine>();
            } else {
                RS_LOGE("parallel_type_[%d] is not supported!\n", parallel_type_);
                return RS_NOT_IMPLEMENT;
            }
            RS_CHECK_PARAM_NULL_RET_STATUS(execute_engine_, "Create executor engine failed!");

            std::vector<NodeWrapper *> run_node_repository;
            for (auto node_wrapper : node_repository_) {
                bool has_input_or_output = false;
                for (auto edge_wrapper : edge_repository_) {
                    if (std::find(edge_wrapper->producers_.begin(), edge_wrapper->producers_.end(),
                                  node_wrapper)
                            != edge_wrapper->producers_.end()
                        || std::find(edge_wrapper->consumers_.begin(),
                                     edge_wrapper->consumers_.end(), node_wrapper)
                               != edge_wrapper->consumers_.end()) {
                        has_input_or_output = true;
                        break;
                    }
                }
                if (has_input_or_output) {
                    run_node_repository.emplace_back(node_wrapper);
                }
            }
            ret = execute_engine_->Init(edge_repository_, run_node_repository);
            RS_RETURN_ON_NEQ(ret, RS_SUCCESS, "executor engine init failed!");

            return ret;
        }
        ErrorCode Graph::Run() {
            ErrorCode ret = RS_SUCCESS;

            // running flag.true,debug or timeprofile use.
            ret = execute_engine_->Run();
            RS_RETURN_ON_NEQ(ret, RS_SUCCESS, "executor engine run failed!");
            // after run ,set flag false
            return ret;
        }

        bool Graph::Synchronize() {
            bool is_synchronize = execute_engine_->Synchronize();
            if (!is_synchronize) {
                RS_LOGE("execute_engine failed!");
            }
            return is_synchronize;
        }

        std::vector<Edge *> Graph::Forward(std::vector<Edge *> inputs) {
            is_forward_api_ok_ = false;
            std::vector<Edge *> outputs;
            return outputs;
        }

        std::vector<Edge *> Graph::operator()(std::vector<Edge *> inputs) {
            if (traced_) {
                ErrorCode ret = this->Run();
                if (ret != RS_SUCCESS) {
                    RS_LOGE("graph run failed!\n");
                    return std::vector<Edge *>();
                }
                return outputs_;
            } else {
                this->MarkInputEdge(inputs);
                std::vector<Edge *> outputs = this->Forward(inputs);
                if (graph_ != nullptr) {
                    ErrorCode ret = graph_->UpdateNodeIO(this, inputs, outputs);
                    if (ret != RS_SUCCESS) {
                        RS_LOGE("graph_->updateNodeIO failed.\n");
                        return std::vector<Edge *>();
                    }
                }
                this->MarkOutputEdge(outputs);
                return outputs;
            }
        }

        std::vector<Edge *> Graph::Forward() {
            Edge *input = nullptr;
            std::vector<Edge *> outputs = this->Forward(input);
            if (IsForwardApiOk()) {
                return outputs;
            } else {
                RS_LOGE("graph[%s] is not implemented forward api!\n", node_name_.c_str());
                is_forward_api_ok_ = false;
                return std::vector<Edge *>();
            }
        }

        std::vector<Edge *> Graph::operator()() {
            if (traced_) {
                ErrorCode ret = this->Run();
                if (ret != RS_SUCCESS) {
                    RS_LOGE("graph run failed!\n");
                    return std::vector<Edge *>();
                }
                return outputs_;
            } else {
                this->MarkInputEdge(std::vector<Edge *>());
                std::vector<Edge *> outputs = this->Forward();
                if (graph_ != nullptr) {
                    ErrorCode ret = graph_->UpdateNodeIO(this, std::vector<Edge *>(), outputs);
                    if (ret != RS_SUCCESS) {
                        RS_LOGE("graph_->updateNodeIO failed.\n");
                        return std::vector<Edge *>();
                    }
                }
                this->MarkOutputEdge(outputs);
                return outputs;
            }
        }

        std::vector<Edge *> Graph::Forward(Edge *input) {
            std::vector<Edge *> inputs;
            inputs.emplace_back(input);
            std::vector<Edge *> outputs = this->Forward(inputs);
            if (IsForwardApiOk()) {
                return outputs;
            } else {
                RS_LOGE("graph[%s] is not implemented forward api!\n", node_name_.c_str());
                is_forward_api_ok_ = false;
                return std::vector<Edge *>();
            }
        }

        std::vector<Edge *> Graph::operator()(Edge *input) {
            if (traced_) {
                ErrorCode ret = this->Run();
                if (ret != RS_SUCCESS) {
                    RS_LOGE("graph run failed!\n");
                    return std::vector<Edge *>();
                }
                return outputs_;
            } else {
                this->MarkInputEdge(std::vector<Edge *>({input}));
                std::vector<Edge *> outputs = this->Forward(input);
                if (graph_ != nullptr) {
                    ErrorCode ret =
                        graph_->UpdateNodeIO(this, std::vector<Edge *>({input}), outputs);
                    if (ret != RS_SUCCESS) {
                        RS_LOGE("graph_->updateNodeIO failed.\n");
                        return std::vector<Edge *>();
                    }
                }
                this->MarkOutputEdge(outputs);
                return outputs;
            }
        }

        bool Graph::IsForwardApiOk() {
            bool ret = is_forward_api_ok_;
            is_forward_api_ok_ = true;
            return ret;
        }

        ErrorCode Graph::AddNodeInputAndOutput(NodeWrapper *node_wrapper,
                                               std::vector<Edge *> inputs,
                                               std::vector<Edge *> outputs) {
            for (auto input : inputs) {
                EdgeWrapper *input_wrapper = FindEdgeWrapper(edge_repository_, input);
                if (input_wrapper == nullptr) {
                    input_wrapper = this->AddEdge(input, true);

                    InsertUnique(input_wrapper->consumers_, node_wrapper);
                }
            }

            for (auto output : outputs) {
                EdgeWrapper *output_wrapper = FindEdgeWrapper(edge_repository_, output);
                if (output_wrapper == nullptr) {
                    output_wrapper = this->AddEdge(output, true);

                    InsertUnique(output_wrapper->producers_, node_wrapper);
                }
            }

            return RS_SUCCESS;
        }

        ErrorCode Graph::Dump(std::ostream &oss) {
            ErrorCode ret = RS_SUCCESS;
            // #TODO
            if (is_inner_) {
            }

            return ret;
        }

        void Graph::SetTraceFlag(bool flag) {
            for (auto node_wrapper : node_repository_) {
                node_wrapper->node_->SetTraceFlag(flag);
            }
        }

        std::vector<Edge *> Graph::Trace() {
            ErrorCode ret = RS_SUCCESS;
            this->SetTraceFlag(true);
            std::vector<Edge *> outputs = this->operator()();

            ret = this->Init();
            if (ret != RS_SUCCESS) {
                RS_LOGE("graph init failed!\n");
                return std::vector<Edge *>();
            }

            traced_ = true;
            return outputs;
        }

        std::vector<Edge *> Graph::Trace(std::vector<Edge *> inputs) {
            ErrorCode ret = RS_SUCCESS;

            this->SetTraceFlag(true);

            std::vector<Edge *> outputs = this->operator()(inputs);

            ret = this->Init();
            if (ret != RS_SUCCESS) {
                RS_LOGE("graph init failed!\n");
                return std::vector<Edge *>();
            }

            traced_ = true;
            return outputs;
        }

        std::vector<Edge *> Graph::Trace(Edge *input) {
            ErrorCode ret = RS_SUCCESS;

            this->SetTraceFlag(true);
            std::vector<Edge *> outputs = this->operator()(input);
            ret = this->Init();
            if (ret != RS_SUCCESS) {
                RS_LOGE("graph init failed!\n");
                return std::vector<Edge *>();
            }

            traced_ = true;
            return outputs;
        }

    } // namespace dag
} // namespace rayshape