#ifndef _DAG_GRAPH_H_
#define _DAG_GRAPH_H_

#include "dag/node.h"
#include "dag/engine.h"
#include "dag/util.h"

// 图可以作为节点嵌入其他图中.  为了让图具备套图(图节点)的能力，让图继承节点,作为graph的子类

// 图能力,具备边的管理能力和节点管理能力
namespace rayshape
{
    namespace dag
    {
        class RS_PUBLIC Graph: public Node {
        public:
            Graph(const std::string &name);

            Graph(const std::string &name, std::vector<Edge *> inputs, std::vector<Edge *> outputs);

            virtual ~Graph();

            /*

             */

            ErrorCode SetInputs(std::vector<Edge *> inputs);
            ErrorCode SetOutputs(std::vector<Edge *> outputs);

            /**
             * 节点管理能力
             */

            // create node.创建节点
            template <typename T, typename... Args,
                      typename std::enable_if<std::is_base_of<Node, T>{}, int>::type = 0>
            Node *CreateNode(const std::string &name, Args &&...args) {
                if (used_node_names_.find(name) != used_node_names_.end()) {
                    RS_LOGE("node name %s already exists\n", name.c_str());
                    return nullptr;
                }
                std::vector<Edge *> inputs;
                std::vector<Edge *> outputs;
                Node *node =
                    dynamic_cast<Node *>(new T(name, inputs, outputs, std::forward<Args>(args)...));
                NodeWrapper *node_wrapper = new NodeWrapper();
                node_wrapper->name_ = name;
                node_wrapper->node_ = node;
                node_repository_.emplace_back(node_wrapper);
                used_node_names_.insert(name);
                // 节点要知道自己归属的图.
                node->SetGraph(this);
                return node;
            } // 用于手动建图
            // single edge creat node override.
            template <typename T, typename... Args,
                      typename std::enable_if<std::is_base_of<Node, T>{}, int>::type = 0>
            Node *CreateNode(const std::string &name, Edge *input, Edge *output, Args &&...args) {
                if (used_node_names_.find(name) != used_node_names_.end()) {
                    RS_LOGE("node name %s already exists\n", name.c_str());
                    return nullptr;
                }
                Node *node = dynamic_cast<Node *>(new T(name, {input}, {output}, args...));
                NodeWrapper *node_wrapper = new NodeWrapper();
                node_wrapper->is_external_ = false;
                node_wrapper->node_ = node;
                node_wrapper->name_ = name;
                EdgeWrapper *input_wrapper = FindEdgeWrapper(edge_repository_, input);
                if (input_wrapper == nullptr) {
                    input_wrapper = this->AddEdge(input);
                }
                input_wrapper->consumers_.emplace_back(node_wrapper);
                EdgeWrapper *output_wrapper = FindEdgeWrapper(edge_repository_, output);
                if (output_wrapper == nullptr) {
                    output_wrapper = this->AddEdge(output);
                }
                output_wrapper->producers_.emplace_back(node_wrapper);

                node_repository_.emplace_back(node_wrapper);
                used_node_names_.insert(name);
                node->SetGraph(this);

                return node;
            }

            template <typename T, typename... Args,
                      typename std::enable_if<std::is_base_of<Node, T>{}, int>::type = 0>
            Node *CreateNode(const std::string &name, std::vector<Edge *> inputs,
                             std::vector<Edge *> outputs, Args &&...args) {
                if (used_node_names_.find(name) != used_node_names_.end()) {
                    RS_LOGE("node name %s already exists\n", name.c_str());
                    return nullptr;
                }

                Node *node = dynamic_cast<Node *>(new T(name, inputs, outputs, args...));
                NodeWrapper *node_wrapper = new NodeWrapper();
                node_wrapper->is_external_ = false;
                node_wrapper->node_ = node;
                node_wrapper->name_ = name;
                for (auto input : inputs) {
                    EdgeWrapper *input_wrapper = FindEdgeWrapper(edge_repository_, input);
                    if (input_wrapper == nullptr) {
                        input_wrapper = this->AddEdge(input);
                    }
                    input_wrapper->consumers_.emplace_back(node_wrapper);
                }

                for (auto output : outputs) {
                    EdgeWrapper *output_wrapper = FindEdgeWrapper(edge_repository_, output);
                    if (output_wrapper == nullptr) {
                        output_wrapper = this->AddEdge(output);
                    }
                    output_wrapper->producers_.emplace_back(node_wrapper);
                }
                node_repository_.emplace_back(node_wrapper);
                used_node_names_.insert(name);

                // 节点要知道自己归属的图.
                node->SetGraph(this); // 在推理时和traced时有用
                return node;
            }

            // add 添加节点
            ErrorCode AddNode(Node *node, bool is_external = true);
            // get node
            // get node by name.
            Node *GetNode(const std::string &name);
            // get node by index
            Node *GetNode(int index);

            // get node count 获取节点数量
            int GetNodeCount() const;
            // recursive get all nodes #TODO

            /*
             *    set node param 设置节点参数 no need
             */

            /**
             * 边管理能力
             */

            // create edge.
            /**
             * @brief 在Graph中创建一条Edge
             * @param  name  edge名称
             * @return Edge*
             */
            Edge *CreateEdge(const std::string &name);

            // add edge
            EdgeWrapper *AddEdge(Edge *edge, bool is_external = true);

            // get edge 获取边
            Edge *GetEdge(const std::string &name);

            // update edge ,更新边,主要是节点外传边进来进行更新
            ErrorCode UpdateEdge(EdgeWrapper *edge_wrapper, Edge *edge, bool is_external = true);

            // 创建节点

            // Node *CreateNode(const std::string &name, Edge *input_edge, Edge *output_edge);

            /**
             * @brief 创建节点
             * @param  name             节点名称
             * @param  input_edge       输入边
             * @param  output_edge      输出边
             * @return Node*
             */
            // ErrorCode SetInput(Edge *input, int index = -1); // 从外部设定指定边

            //  update node io
            ErrorCode UpdateNodeIO(Node *node, std::vector<Edge *> inputs,
                                   std::vector<Edge *> outputs);

            ErrorCode MarkInputEdge(std::vector<Edge *> inputs);
            ErrorCode MarkOutputEdge(std::vector<Edge *> outputs);

            /**
             * @brief graph 初始化,继承自node
             */
            virtual ErrorCode Init() override;

            /**
             * @brief graph 反初始化,继承自node
             */
            virtual ErrorCode Deinit() override;

            /**
             * @brief graph 运行 对node父类进行重载
             */
            virtual ErrorCode Run() override;

            // 下面的虚方法必须被子类重载
            // 子类应该重载这些方法去定义他们自己的运算符() 执行,即子图的节点运行逻辑
            virtual std::vector<Edge *> Forward(std::vector<Edge *> inputs); // muti input edge
            virtual std::vector<Edge *> operator()(std::vector<Edge *> inputs);
            virtual std::vector<Edge *> Forward(); // no input
            virtual std::vector<Edge *> operator()();
            virtual std::vector<Edge *> Forward(Edge *input); // single input
            virtual std::vector<Edge *> operator()(Edge *input);
            /**
             * @brief graph 图结构信息打印 only for graph not for node
             */
            ErrorCode Dump(std::ostream &oss);
            // 子类可以访问并可以重载带virtual的虚函数

            /*
             *trace 用于非子图的运行，用于动态trace的建立图
             */
            virtual void SetTraceFlag(bool flag);
            std::vector<Edge *> Trace(std::vector<Edge *> inputs);
            std::vector<Edge *> Trace();
            std::vector<Edge *> Trace(Edge *input);

            bool IsForwardApiOk();

        protected:
            /**
             *依据节点的生产消费关系构建图
             */
            virtual ErrorCode Construct(); // 图构建函数 //在init函数中进行节点和边的串联

            // virtual ErrorCode Executor();

            // 迭代节点仓库,获取到节点的所有输入边，输入边找到包装边(有生产消费关系),由此构建节点的前驱后继关系
            // 迭代边,给边增加生产着和消费者,节点的边是有类型的;图的类型决定边的类型
            virtual ErrorCode InitExecuteEngine(); // 依据节点并行的情况初始化引擎具体的执行情况.
            // protected 属性对于继承类可见，实现特例化的图

        private:
            // helper function
            /**
             * 帮助节点和边建立关系
             *
             *
             */
            ErrorCode AddNodeInputAndOutput(NodeWrapper *node_wrapper, std::vector<Edge *> inputs,
                                            std::vector<Edge *> outputs);

        protected:
            std::vector<EdgeWrapper *> edge_repository_;
            // one graph has many edges build a repository.
            std::vector<NodeWrapper *> node_repository_;
            // one graph has many nodes build a repository.

            std::set<std::string> used_edge_names_; // 用过的边命名,创建边时,不能重复使用
            std::set<std::string> used_node_names_; //  用过的节点命名,创建节点时,不能重复使用

            std::shared_ptr<ExecuteEngine> execute_engine_; // 调度引擎

            bool is_forward_api_ok_ = true;
            // std::vector<std::shared_ptr<Edge>> shared_edge_repository_;
            // std::vector<std::shared_ptr<Node>> shared_node_repository_;
        };

    } // namespace dag
} // namespace rayshape

#endif