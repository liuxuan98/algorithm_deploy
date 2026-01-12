#include "dag/node.h"
#include "dag/graph.h"

namespace rayshape
{
    namespace dag
    {
        Node::Node(const std::string &name) {
            if (name.empty()) {
                // 随机命名
                // name_ = "node_" + ;
            } else {
                node_name_ = name;
            }
            // constructed_ = true;
        }

        Node::Node(const std::string &name, std::vector<Edge *> inputs,
                   std::vector<Edge *> outputs) {
            if (name.empty()) {
                // 随机命名
            } else {
                node_name_ = name;
            }
            // 节点可以有多个输入边
            for (auto input : inputs) {
                for (auto output : outputs) {
                    if (input == output) {
                        RS_LOGW("Input edge[%s] is same as output edge[%s].\n",
                                input->GetName().c_str(), output->GetName().c_str());
                    }
                }
            }

            inputs_ = inputs;
            outputs_ = outputs;
            is_constructed_ = true;
        }

        Node::~Node() {
            if (is_init_ == true) {
                this->Deinit();
            }

            inputs_.clear();
            outputs_.clear();

            is_constructed_ = false;
            is_init_ = false;
        }

        void Node::SetName(const std::string &name) {
            node_name_ = name;
        }

        std::string Node::GetName() const {
            return node_name_;
        }

        void Node::SetGraph(Graph *graph) {
            graph_ = graph;
        }

        Graph *Node::GetGraph() {
            return graph_;
        }

        ErrorCode Node::SetInputs(std::vector<Edge *> inputs) {
            inputs_ = inputs;
            return RS_SUCCESS;
        }

        ErrorCode Node::SetOutputs(std::vector<Edge *> outputs) {
            outputs_ = outputs;
            return RS_SUCCESS;
        }

        Edge *Node::CreateInternalOutputEdge(const std::string &name) {
            Edge *edge = new Edge(name);
            bool is_exist = false;
            for (int i = 0; i < outputs_.size(); i++) {
                if (outputs_[i]->GetName() == name) {
                    is_exist = true;
                    outputs_[i] = edge;
                    break;
                }
            }
            if (!is_exist) {
                outputs_.emplace_back(edge);
            }
            return edge;
        }

        Edge *Node::GetInputEdge(int index) {
            if (index >= 0 && index < inputs_.size()) {
                return inputs_[index];
            }

            return nullptr;
        }

        Edge *Node::GetOutputEdge(int index) {
            if (index >= 0 && index < outputs_.size()) {
                return outputs_[index];
            }

            return nullptr;
        }

        std::vector<Edge *> Node::GetAllInput() {
            return inputs_;
        }

        std::vector<Edge *> Node::GetAllOutput() {
            return outputs_;
        }

        ErrorCode Node::SetParallelType(const ParallelType paralle_type) {
            parallel_type_ = parallel_type_ == paralle_type ? parallel_type_ : paralle_type;
            return RS_SUCCESS;
        }

        ParallelType Node::GetParallelType() {
            return parallel_type_;
        }

        bool Node::GetConstructed() {
            return is_constructed_;
        }

        void Node::SetInitStatus(bool status) {
            is_init_ = status;
        }

        bool Node::GetInitStatus() const {
            return is_init_;
        }

        void Node::SetTraceFlag(bool flag) {
            is_trace_ = flag;
        }

        ErrorCode Node::Init() {
            SetInitStatus(true);
            return RS_SUCCESS;
        }

        ErrorCode Node::Deinit() {
            SetInitStatus(false);
            return RS_SUCCESS;
        }

        std::vector<Edge *> Node::Forward(std::vector<Edge *> inputs) {
            // init
            // 节点未初始化并且不设置的trace flag进行边串联trace.
            if (is_init_ == false && is_trace_ == false) {
                // this->SetInitStatus();
                this->Init();
            }
            // check nndeploy will check ton
            if (!this->CheckInputs(inputs)) {
                return std::vector<Edge *>();
            }

            bool is_inputs_changed = this->IsInputsChanged(inputs);
            if (!inputs.empty()) {
                this->SetInputs(inputs);
            }
            std::vector<std::string> real_outputs_name =
                this->GetRealOutputsName(); // nndeploy 针对根据type信息构造边
            std::vector<Edge *> outputs;
            // 节点operation.
            for (const auto &name : real_outputs_name) {
                Edge *edge = nullptr;
                if (graph_ != nullptr) { // 节点所在的图中找输入边,这里有点疑惑
                    edge = graph_->GetEdge(name);
                    if (edge != nullptr) {
                        outputs.push_back(edge);
                    }
                }
                if (edge == nullptr) {
                    // creat inner output edge.//todo
                    edge = this->CreateInternalOutputEdge(name);
                    if (edge != nullptr) {
                        outputs.push_back(edge);
                    } else {
                        RS_LOGE("createInternalOutputEdge failed.\n");
                        return std::vector<Edge *>();
                    }
                }
            }

            if (!outputs.empty()) {
                this->SetOutputs(outputs);
            }
            // 是父图还是代表这个节点是个图要打一个问号
            if (graph_ != nullptr) {
                ErrorCode ret = graph_->UpdateNodeIO(this, inputs, outputs);
                if (ret != RS_SUCCESS) {
                    RS_LOGE("graph_->updateNodeIO failed.\n");
                    return std::vector<Edge *>();
                }
            }
            // 输入没变化且节点是trace,直接返回outputs
            if (!is_inputs_changed && is_trace_) {
                return outputs;
            } else {
                ErrorCode ret = this->Run(); // 节点运行重载函数
                if (ret != RS_SUCCESS) {
                    RS_LOGE("node[%s] run failed.\n", this->GetName().c_str());
                    return std::vector<Edge *>();
                }
                return outputs;
            }
        }

        std::vector<Edge *> Node::operator()(std::vector<Edge *> inputs) {
            return this->Forward(inputs);
        }

        std::vector<Edge *> Node::Forward() {
            return this->Forward(std::vector<Edge *>());
        }

        std::vector<Edge *> Node::operator()() {
            return this->Forward();
        }

        std::vector<Edge *> Node::Forward(Edge *input) {
            return this->Forward(std::vector<Edge *>({input}));
        }

        std::vector<Edge *> Node::operator()(Edge *input) {
            return this->Forward(input);
        }

        bool Node::CheckInputs(std::vector<Edge *> &inputs) {
#if 0
            if (input_type_info_.empty()) {
                return true;
            }
            if (inputs.size() == input_type_info_.size()) {
                for (size_t i = 0; i < inputs.size(); i++) {
                    if (inputs[i]->getTypeInfo() != *(input_type_info_[i])) {
                        return false;
                    }
                }
                return true;
            }
            RS_LOGE("inputs.size()[%d] != input_type_info_.size()[%d]\n", inputs.size(),
                          input_type_info_.size());
            return false;
#endif
            return true;
        }

        bool Node::IsInputsChanged(std::vector<Edge *> inputs) {
            if (inputs_.empty()) { // 输入边为空则没有改变
                return false;
            }
            if (inputs.size() != inputs_.size()) {
                return true;
            }
            for (size_t i = 0; i < inputs.size(); i++) {
                if (inputs[i] != inputs_[i]) {
                    return true;
                }
            }

            return false;
        }

        std::vector<std::string> Node::GetRealOutputsName() {
            std::vector<std::string> real_outputs_name; // 主动构建边
            if (!outputs_.empty()) {
                for (int i = 0; i < outputs_.size(); i++) {
                    real_outputs_name.push_back(outputs_[i]->GetName());
                }
            } else {
                // get
                RS_LOGE("Node[%s] has not outputs.\n", this->GetName().c_str());
                return std::vector<std::string>();
            }

            return real_outputs_name;
        }

        void Node::SetRunningFlag(bool flag) {
            if (flag) {
                run_size_++;
            } else if (flag == false && is_running_) {
                completed_size_++;
            }
            is_running_ = flag;
        }

        size_t Node::GetRunCompletedSize() {
            return completed_size_;
        }

        bool Node::Synchronize() {
            return true;
        }

        EdgeUpdateFlag Node::UpdateInput() {
            EdgeUpdateFlag flag = EdgeUpdateFlag::Complete;
            for (auto input : inputs_) {
                flag = input->Update(this);
                if (flag != EdgeUpdateFlag::Complete) {
                    break;
                }
            }
            return flag;
        }

    } // namespace dag
} // namespace rayshape