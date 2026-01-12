#include "dag/util.h"

namespace rayshape
{
    namespace dag
    {

        std::vector<NodeWrapper *> CheckUnuseNode(std::vector<NodeWrapper *> &node_repository) {
            std::vector<NodeWrapper *> unused; //依据颜色标志位来确定无用节点,graph 
            for (auto node_wrapper : node_repository) {
                if (node_wrapper->color_ == NODE_COLOR_WHITE) {
                    RS_LOGD("Unuse node found in graph, Node name: %s.\n",
                            node_wrapper->name_.c_str());
                    unused.push_back(node_wrapper);
                }
            }
            return unused;
        }

        std::vector<NodeWrapper *> FindStartNodes(std::vector<NodeWrapper *> &node_repository) {
            std::vector<NodeWrapper *> start_nodes;
            for (auto node_wrapper : node_repository) {
                if (node_wrapper->predecessors_.empty()) {
                    start_nodes.emplace_back(node_wrapper);
                }
            }
            return start_nodes;
        }
        // kahn算法/BFS 实现拓扑排序.层序遍历
        ErrorCode TopoSortBFS(std::vector<NodeWrapper *> &node_repository,
                              std::vector<NodeWrapper *> &topo_sort_node) {
            // 1.找到所有入度为0的节点作为起始节点
            std::vector<NodeWrapper *> start_nodes = FindStartNodes(node_repository);
            if (start_nodes.empty()) {
                RS_LOGE("No start node found in graph!\n");
                return RS_INVALID_PARAM_VALUE;
            }

            // 2.记录每个节点的入度
            std::unordered_map<NodeWrapper *, int> node_in_degree;
            for (auto node : node_repository) {
                node_in_degree[node] = node->predecessors_.size(); // node 前驱节点的数量.
            }

            // 3.将所有入度为0的节点加入队列
            std::queue<NodeWrapper *> q;
            for (auto node : start_nodes) {
                q.push(node);
            }
            int max_width = 0; // 最大宽度,表示最大的并发任务量可以设置线程？
            // 4.BFS遍历,广度优先搜索.

            while (!q.empty()) {
                int level_size = q.size();
                max_width = std::max(max_width, level_size); // record and update max level node
                                                             // num;

                NodeWrapper *cur_node = q.front();
                q.pop();
                cur_node->color_ = NODE_COLOR_BLACK;
                topo_sort_node.emplace_back(cur_node);

                // 当前节点的所有后继节点的入度减1
                for (auto next_node : cur_node->successors_) {
                    node_in_degree[next_node]--;
                    // 如果入度变为0,则加入队列
                    if (node_in_degree[next_node] == 0) {
                        q.push(next_node);
                    }
                }
            }

            // 检查是否成环, Kahn 拓扑排序算法，若有环必定有节点无法入队,因此最终输出节点会少于输入节点
            if (topo_sort_node.size() != node_repository.size()) {
                RS_LOGE("Cycle detected in graph!\n");
                return RS_INVALID_PARAM_VALUE;
            }
            // check unused node.
            CheckUnuseNode(node_repository);

            return RS_SUCCESS;
        }

        ErrorCode DFSRecursive(NodeWrapper *node, std::stack<NodeWrapper *> &dst) {
            ErrorCode ret = RS_SUCCESS;

            node->color_ = NODE_COLOR_GRAY;
            for (auto next_node : node->successors_) {
                // 节点未访问,递归访问
                if (next_node->color_ == NODE_COLOR_WHITE) {
                    ret = DFSRecursive(next_node, dst);
                    if (ret != RS_SUCCESS) {
                        RS_LOGE("Cycle detected in graph!\n");
                        return ret;
                    }
                    // 节点正访问，有环
                } else if (next_node->color_ == NODE_COLOR_GRAY) {
                    RS_LOGE("Cycle detected in graph!\n");
                    return RS_INVALID_PARAM_VALUE;
                } else {
                    // 节点访问过,跳出
                    continue;
                }
            }

            node->color_ = NODE_COLOR_BLACK;
            dst.push(node);
            return ret;
        }
        ErrorCode TopoSortDFS(std::vector<NodeWrapper *> &node_repository,
                              std::vector<NodeWrapper *> &topo_sort_node) {
            ErrorCode ret = RS_SUCCESS;
            std::vector<NodeWrapper *> start_nodes = FindStartNodes(node_repository);
            if (start_nodes.empty()) {
                RS_LOGE("Can not find start node in graph\n");
                return RS_INVALID_PARAM_VALUE;
            }
            std::stack<NodeWrapper *> dst;
            for (auto node_wrapper : start_nodes) {
                if (node_wrapper->color_ == NODE_COLOR_WHITE) {
                    ret = DFSRecursive(node_wrapper, dst);
                    if (ret != RS_SUCCESS) {
                        RS_LOGE("Cycle detected in graph\n");
                        return ret;
                    }
                } else if (node_wrapper->color_ == NODE_COLOR_GRAY) {
                    RS_LOGE("Cycle detected in graph\n");
                    return RS_INVALID_PARAM_VALUE;
                } else {
                    continue;
                }
            }
            // 节点逆序排序
            while (!dst.empty()) {
                topo_sort_node.emplace_back(dst.top());
                dst.pop();
            }

            CheckUnuseNode(node_repository);

            return ret;
        }

        EdgeWrapper *FindEdgeWrapper(std::vector<EdgeWrapper *> &edge_repository, Edge *edge) {
            for (auto edge_wrapper : edge_repository) {
                if (edge_wrapper->edge_ == edge) {
                    return edge_wrapper;
                }
            }
            return nullptr;
        }

        EdgeWrapper *FindEdgeWrapper(std::vector<EdgeWrapper *> &edge_repository,
                                     const std::string &edge_name) {
            for (auto edge_wrapper : edge_repository) {
                if (edge_wrapper->name_ == edge_name) {
                    return edge_wrapper;
                }
            }
            return nullptr;
        }

        ErrorCode DumpDag(std::vector<EdgeWrapper *> &edge_repository,
                          std::vector<NodeWrapper *> &node_repository,
                          std::vector<Edge *> &graph_inputs, std::vector<Edge *> &graph_outputs,
                          const std::string &name, std::ostream &oss) {
            return RS_SUCCESS;
        }

    } // namespace dag
} // namespace rayshape
