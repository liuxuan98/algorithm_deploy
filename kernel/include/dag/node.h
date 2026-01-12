#ifndef _DAG_NODE_H_
#define _DAG_NODE_H_

#include "base/error.h"
#include "base/glic_stl_include.h"
#include "dag/edge.h"

// 节点和图同根同源
// node contrl the input and output edge
// node init before,the input/output edge can be ensure;
namespace rayshape
{ // 类似与Cgraph中的Gelements
    namespace dag
    {
        class Graph; //

        // 抽象基类
        class RS_PUBLIC Node {
        public:
            Node(const std::string &name);

            Node(const std::string &name, std::vector<Edge *> inputs, std::vector<Edge *> outputs);
            // 节点有入边和出边,可以多条;但一条边只能link两个节点.

            virtual ~Node();

            void SetName(const std::string &name);

            std::string GetName() const;

            /*
            节点运行接口
            */
            void SetGraph(Graph *graph);

            /*
             *返回graph 指针
             */
            Graph *GetGraph();
            // 考虑private属性对内API即可
            virtual ErrorCode SetInputs(std::vector<Edge *> inputs);

            virtual ErrorCode SetOutputs(std::vector<Edge *> outputs);

            virtual Edge *CreateInternalOutputEdge(const std::string &name);
            /*
             * node edge manager
             */
            Edge *GetInputEdge(int index = 0);

            Edge *GetOutputEdge(int index = 0);

            std::vector<Edge *> GetAllInput();

            std::vector<Edge *> GetAllOutput();

            virtual ErrorCode SetParallelType(const ParallelType paralle_type);

            virtual ParallelType GetParallelType();

            // pipeline all node attribute.
            void SetRunningFlag(bool flag);

            size_t GetRunCompletedSize();

            bool Synchronize();
            //

            // set all kind of status.

            bool GetConstructed();

            void SetInitStatus(bool status);

            bool GetInitStatus() const;

            /**
             * 设置SetTrace
             */
            virtual void SetTraceFlag(bool flag);

            virtual ErrorCode Init(); // 初始化函数

            virtual ErrorCode Deinit(); // 释放函数

            virtual EdgeUpdateFlag UpdateInput(); // 更新输入函数

            virtual ErrorCode Run() = 0; // 流程处理函数

            // virtual bool Synchronize();

            /**
             * @brief 节点调用接口
             * @details 节点调用接口，用于节点之间的调用
             * @param inputs 输入的边
             * @return 返回的边
             * @note
             * 1. 存在graph，返回值有graph管理
             * 2. 不存在graph，返回值由node管理
             */

            virtual std::vector<Edge *> Forward(std::vector<Edge *> inputs);
            virtual std::vector<Edge *> operator()(std::vector<Edge *> inputs);
            virtual std::vector<Edge *> Forward();
            virtual std::vector<Edge *> operator()();
            virtual std::vector<Edge *> Forward(Edge *input);
            virtual std::vector<Edge *> operator()(Edge *input);

            bool CheckInputs(std::vector<Edge *> &inputs);
            bool IsInputsChanged(std::vector<Edge *> inputs);
            // 考虑放到real output nodes
            virtual std::vector<std::string> GetRealOutputsName();

        protected:
            /** 配置相关信息 */
            std::string node_name_;
            // 是否是图中的内部节点.
            bool is_inner_ = false;

            /** 状态相关信息 */
            bool is_init_ = false;
            bool is_constructed_ = false;

            /** 图相关信息 */
            std::vector<Edge *> inputs_; // 图作为node包含的图的输入输出边的信息
            std::vector<Edge *> outputs_;
            Graph *graph_ = nullptr;

            /** 节点执行期间的相关信息 */
            bool is_prepared_ = false;
            // 参数map,可以用边传递数据就行

            //
            ParallelType parallel_type_ = ParallelType::PARALLEL_TYPE_NONE;

        protected:
            bool is_trace_ = false; // 节点是否被追踪
            bool traced_ = false;   // 图节点是否被追踪

            size_t completed_size_ = 0;
            size_t run_size_ = 0;
            bool is_running_ = false;

            // Graph *graph_ = nullptr; // node 得知道在哪个图里

            // bool constructed_ = false; // node 是否已经构造完成

            // bool initialized_ = false; // node 是否已经初始化

            // Param param_ = nullptr; param 存储任意参数信息
        };

        // 节点注册机制暂时不引入

        /**
         * @brief 节点注册机制相关类和函数
         */
        // NodeCreator

    } // namespace dag

} // namespace rayshape

#endif

// 图节点基类
