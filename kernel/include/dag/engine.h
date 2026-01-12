#ifndef _DAG_ENGINE_H_
#define _DAG_ENGINE_H_

#include "base/macros.h"
#include "util.h"
#include "base/error.h"
#include "base/common.h"

// 进行图节点调度的基类引擎
// 动态调度、运行时调度,静态节点
// 不可以复制构造和拷贝
namespace rayshape
{
    namespace dag
    {
        class RS_PUBLIC ExecuteEngine: public NonCopyable {
        public:
            ExecuteEngine() {};

            virtual ~ExecuteEngine() {};

            virtual ErrorCode Init(
                std::vector<EdgeWrapper *> &edge_repository,
                std::vector<NodeWrapper *> &node_repository) = 0; // node and edge

            virtual ErrorCode Setup() = 0;
            // build graph and 判断是否是Dag,分析几个输入输出,拓扑排序

            virtual ErrorCode DeInit() = 0;

            virtual ErrorCode Run() = 0;

        private:
            void *m_engine = nullptr;
        };

    } // namespace dag
} // namespace rayshape

#endif