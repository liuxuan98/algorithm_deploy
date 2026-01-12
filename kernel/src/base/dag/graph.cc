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

        // 添加外部节点
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
            // 获取添加节点的输入输出边.
            for (auto input : node->GetAllInput()) {
                EdgeWrapper *input_wrapper = FindEdgeWrapper(edge_repository_, input);
                if (input_wrapper == nullptr) {
                    input_wrapper = this->AddEdge(input, true);
                }
                // 给node edge 进行wrap后 给wrap 边添加消费者,因为是输入边
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

            node->SetGraph(this); // node know which graph it belongs to 添加所属关系

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
            // auto shared_it = std::find_if();
            //  #todo.
            return RS_SUCCESS;
        }

        /**
         *end edge manager API
         */

        // ErrorCode Graph::SetInput(Edge *input, int index) {
        //     if (input == nullptr) {
        //         RS_LOGE("");
        //     }
        // }
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
                // 添加生产者
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

            // set init flag 设置初始化标志
            SetInitStatus(true);

            return ret;
        }

        ErrorCode Graph::Deinit() {
            ErrorCode ret = RS_SUCCESS;

            if (execute_engine_ != nullptr) {
                ret = execute_engine_->DeInit();
                RS_RETURN_ON_NEQ(ret, RS_SUCCESS, "execute_engine_ deinit failed!");
            } else {
                // 节点是否需要初始化标志位，个人感觉在process中也不怎么用到的
                for (auto node_wrapper : node_repository_) {
                }
            }

            // 图节点init标志位置为false

            return ret;
        }

        ErrorCode Graph::Construct() {
            ErrorCode ret = RS_SUCCESS;

            for (auto node_wrapper : node_repository_) {
                // 给节点添加生产消费关系的边.(已节点为主),建立后就可以建立节点之间的关系(前驱后继关系)
                RS_CHECK_PARAM_NULL_RET_STATUS(node_wrapper->node_, "node_wrapper node is null!");
                std::vector<Edge *> inputs = node_wrapper->node_->GetAllInput();
                std::vector<Edge *> outputs = node_wrapper->node_->GetAllOutput();
                this->AddNodeInputAndOutput(node_wrapper, inputs, outputs);
            }

            for (auto edge_wrapper : edge_repository_) {
                RS_CHECK_PARAM_NULL_RET_STATUS(edge_wrapper->edge_,
                                               "edge_repository_ edge is null!");
            }
            // 给边添加生产者消费者关系.
            for (auto node_wrapper : node_repository_) {
                Node *node = node_wrapper->node_;
                // 可以给单节点设定一些特定的运行参数 debug timeprofile 进行debug或单节点的性能计时
                node->SetParallelType(parallel_type_); // 设定节点的并行类型
                std::vector<Edge *> inputs = node->GetAllInput();
                for (auto input : inputs) {
                    EdgeWrapper *in_edge_wrapper = FindEdgeWrapper(edge_repository_, input);
                    RS_CHECK_PARAM_NULL_RET_STATUS(in_edge_wrapper, "in_edge_wrapper is null!");
                    for (auto producer : in_edge_wrapper->producers_) { // 边的生产者，即前驱节点
                        // 节点通过输入边可以得知该边的生产者,即改节点的前驱节点
                        InsertUnique(node_wrapper->predecessors_,
                                     producer); // 给该节点的添加前驱节点.
                    }
                }

                std::vector<Edge *> outputs = node->GetAllOutput();
                // 同理输出边也是,通过输出边添加后继节点
                for (auto output : outputs) {
                    EdgeWrapper *out_edge_wrapper = FindEdgeWrapper(edge_repository_, output);
                    RS_CHECK_PARAM_NULL_RET_STATUS(out_edge_wrapper, "out_edge_wrapper is null!");
                    for (auto consumer : out_edge_wrapper->consumers_) {
                        InsertUnique(node_wrapper->successors_, consumer);
                    }
                }
            }
            // 以上已基本建立节点关系
            //  同时边也管理自己的生产消费者关系
            for (auto edge_wrapper : edge_repository_) {
                std::vector<Node *> producers;
                for (auto producer : edge_wrapper->producers_) {
                    producers.emplace_back(producer->node_);
                }

                std::vector<Node *> consumers;
                for (auto consumer : edge_wrapper->consumers_) {
                    consumers.emplace_back(consumer->node_);
                }

                // edge抽象边 用的桥接mode(我们只有fixed边用于并行任务调度)
                // 因

                ErrorCode ret = edge_wrapper->edge_->SetParallelType(parallel_type_);
                RS_RETURN_ON_NEQ(ret, RS_SUCCESS, "setParallelType failed!");

                /**
                 * abstract edge管理该字段 边的抽象
                 *
                 * */
                ret = edge_wrapper->edge_->IncreaseProducers(producers);
                RS_RETURN_ON_NEQ(ret, RS_SUCCESS, "edge IncreaseProducers failed!");

                ret = edge_wrapper->edge_->IncreaseConsumers(consumers);
                RS_RETURN_ON_NEQ(ret, RS_SUCCESS, "edge IncreaseConsumers failed!");
                // 边的构建
                ret = edge_wrapper->edge_->Construct();
                RS_RETURN_ON_NEQ(ret, RS_SUCCESS, "edge construct failed!");

                // pipeline use
                // ret = edge_wrapper->edge_->setQueueMaxSize(queue_max_size_)
                // RS_RETURN_ON_NEQ(ret, RS_SUCCESS, "edge setQueueMaxSize failed!");
            }

            // #TODO:引入cuda流 对于节点的运行

            // 对于图来说没有生产者的为输入边
            for (auto edge_wrapper : edge_repository_) {
                if (edge_wrapper->producers_.empty()) {
                    // 如果找不到
                    auto it = std::find(inputs_.begin(), inputs_.end(), edge_wrapper->edge_);
                    if (it == inputs_.end()) {
                        inputs_.emplace_back(edge_wrapper->edge_);
                    }
                }
            }

            // 没有消费者的为输出边

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
                    // iter->MarkGraphOutput();  todo.
                }
                // 该图输出确实为最终图输出
            }

            return ret;
        }
        // 两种方式
        ErrorCode Graph::InitExecuteEngine() {
            ErrorCode ret = RS_SUCCESS;
            // 创建图的调度器
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
            // 将是生产者或消费者的节点(统称运行节点)收集起来,(用处剔除无用node?),后面加入执行调度器中
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
            // 执行调度器初始化,会对节点进行拓扑排序
            ret = execute_engine_->Init(edge_repository_, run_node_repository);
            RS_RETURN_ON_NEQ(ret, RS_SUCCESS, "executor engine init failed!");

            return ret;
        }
        // runing标志位
        ErrorCode Graph::Run() {
            ErrorCode ret = RS_SUCCESS;

            // 是否需要设置running flag.true
            ret = execute_engine_->Run();
            RS_RETURN_ON_NEQ(ret, RS_SUCCESS, "executor engine run failed!");
            // 运行完设置flag false
            return ret;
        }
        // 继承图必须被重载Forward的这个接口
        std::vector<Edge *> Graph::Forward(std::vector<Edge *> inputs) {
            is_forward_api_ok_ = false;
            std::vector<Edge *> outputs;
            return outputs;
        }

        std::vector<Edge *> Graph::operator()(std::vector<Edge *> inputs) {
            if (traced_) {
                // 跑一轮
                ErrorCode ret = this->Run(); // 图运行结果引擎会直接调度已经组织好的图
                if (ret != RS_SUCCESS) {
                    RS_LOGE("graph run failed!\n");
                    return std::vector<Edge *>();
                }
                return outputs_; // 顺序图直接这样获取结果
            } else {
                this->MarkInputEdge(inputs); // 可以做void
                std::vector<Edge *> outputs = this->Forward(inputs);
                if (graph_ != nullptr) { // 该图是别个图(父图)的子图
                    ErrorCode ret = graph_->UpdateNodeIO(this, inputs, outputs);
                    if (ret != RS_SUCCESS) { // 父图？
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
            // 追踪只会追踪一次
            if (traced_) {
                ErrorCode ret = this->Run(); // 图运行结果引擎会直接调度已经组织好的图
                if (ret != RS_SUCCESS) {
                    RS_LOGE("graph run failed!\n");
                    return std::vector<Edge *>();
                }
                return outputs_;
            } else {
                this->MarkInputEdge(std::vector<Edge *>()); // 可以做void
                std::vector<Edge *> outputs = this->Forward();
                if (graph_ != nullptr) { // 该图是别个图(父图)的子图
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
            if (traced_) {                   // 动态构图完成会直接跑图
                ErrorCode ret = this->Run(); // 图运行结果引擎会直接调度已经组织好的图
                if (ret != RS_SUCCESS) {
                    RS_LOGE("graph run failed!\n");
                    return std::vector<Edge *>();
                }
                return outputs_;
            } else {
                this->MarkInputEdge(std::vector<Edge *>({input}));
                std::vector<Edge *> outputs = this->Forward(input);
                if (graph_ != nullptr) { // 该图是别个图(父图)的子图
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

        // 针对每个节点的边的建立生产消费关系
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

            // start todo
            if (is_inner_) {
            }

            return ret;
        }

        // 动态构图,动态构图的跟踪轨迹?

        void Graph::SetTraceFlag(bool flag) {
            // 图节点仓库在图构建函数就已经建立了,在trace之前做完.
            for (auto node_wrapper : node_repository_) {
                node_wrapper->node_->SetTraceFlag(flag);
            }
        }

        std::vector<Edge *> Graph::Trace() {
            ErrorCode ret = RS_SUCCESS;
            this->SetTraceFlag(true); // 节点跟踪
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

            std::vector<Edge *> outputs =
                this->operator()(inputs); // 操作符会调用forward通过节点的流程.

            ret = this->Init(); // dynamic graph 初始化构图
            if (ret != RS_SUCCESS) {
                RS_LOGE("graph init failed!\n");
                return std::vector<Edge *>();
            }

            traced_ = true; // 已跟踪完成了一遍,动态构图已经完成.
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