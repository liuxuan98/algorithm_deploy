#ifndef _DAG_UTIL_H_
#define _DAG_UTIL_H_

#include <random>

#include "base/macros.h"
#include "base/error.h"
#include "dag/node.h"
#include "dag/edge.h"
#include "base/common.h"

namespace rayshape
{
    namespace dag
    {
        // 节点可以有处理状态标志位置,来传导下一个节点
        class RS_PUBLIC NodeWrapper {
        public:
            bool is_external_;
            Node *node_; // real node.
            std::string name_;
            std::vector<NodeWrapper *> predecessors_; // 前驱
            std::vector<NodeWrapper *> successors_;   // 后继

            NodeColorType color_ = NODE_COLOR_WHITE; // topology排序三色法
        };

        class RS_PUBLIC EdgeWrapper {
        public:
            bool is_external_;
            Edge *edge_; // 边可以持有任何数据包,边需不需要状态
            std::string name_;
            std::vector<NodeWrapper *> producers_; // 该条边的生产者
            std::vector<NodeWrapper *> consumers_; // 该条边的消费者
        };

        std::vector<NodeWrapper *> CheckUnuseNode(std::vector<NodeWrapper *> &node_repository);

        std::vector<NodeWrapper *> FindStartNodes(std::vector<NodeWrapper *> &node_repository);

        ErrorCode TopoSortBFS(std::vector<NodeWrapper *> &node_repository,
                              std::vector<NodeWrapper *> &topo_sort_node);

        ErrorCode TopoSortDFS(std::vector<NodeWrapper *> &node_repository,
                              std::vector<NodeWrapper *> &topo_sort_node);

        static std::string GetUniqueString() {
            // Add timestamp
            auto now = std::chrono::system_clock::now();
            auto timestamp =
                std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch())
                    .count();

            // Generate random number
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, 999999);
            int random = dis(gen);

            // Combine to generate unique string
            std::stringstream ss;
            ss << timestamp << "_" << random;
            return ss.str();
        }

        /**
         * @brief 对vector插入不在vector中的元素，即类似集合的作用
         * @tparam T
         * @param  vec              My Param doc
         * @param  val              My Param doc
         */
        template <typename T>
        void InsertUnique(std::vector<T> &vec, const T &val) {
            if (std::find(vec.begin(), vec.end(), val) == vec.end()) {
                vec.emplace_back(val);
            }
        }

        /**
         * 从众多边warpper仓库中找到指定边的warpper
         */
        RS_PUBLIC EdgeWrapper *FindEdgeWrapper(std::vector<EdgeWrapper *> &edge_repository,
                                               Edge *edge); //符号未导出

        RS_PUBLIC EdgeWrapper *FindEdgeWrapper(std::vector<EdgeWrapper *> &edge_repository,
                                     const std::string &edge_name);
        // dump DAG
        ErrorCode DumpDag(std::vector<EdgeWrapper *> &edge_repository,
                          std::vector<NodeWrapper *> &node_repository,
                          std::vector<Edge *> &graph_inputs, std::vector<Edge *> &graph_outputs,
                          const std::string &name, std::ostream &oss);
    } // namespace dag
} // namespace rayshape

#endif