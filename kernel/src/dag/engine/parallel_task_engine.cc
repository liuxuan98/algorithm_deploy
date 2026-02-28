#include "dag/engine/parallel_task_engine.h"
#include "dag/util.h"

namespace rayshape
{
    namespace dag
    {
        ParallelTaskEngine::ParallelTaskEngine() : ExecuteEngine() {
            thread_pool_ = nullptr;
            all_task_count_ = 0;
            global_status_ = RS_SUCCESS;
        }

        ParallelTaskEngine::~ParallelTaskEngine() {}

        ErrorCode ParallelTaskEngine::Init(std::vector<EdgeWrapper *> &edge_repository,
                                           std::vector<NodeWrapper *> &node_repository) {
            ErrorCode ret = RS_SUCCESS;

            thread_pool_ = new threadpool::ThreadPool();
            thread_pool_->Init();

            start_nodes_ = FindStartNodes(node_repository);
            if (start_nodes_.empty()) {
                RS_LOGE("No start node found in graph.\n");
                return RS_INVALID_PARAM_VALUE;
            }
            ret = TopoSortBFS(node_repository, topo_sort_node_);
            if (ret != RS_SUCCESS) {
                RS_LOGE("TopoSortBFS Failed!\n");
                return ret;
            }
            all_task_count_ = static_cast<int>(topo_sort_node_.size());

            for (auto iter : topo_sort_node_) {
                // node init
                iter->color_ = NODE_COLOR_WHITE;
                if (iter->node_->GetInitStatus()) {
                    continue;
                }

                ret = iter->node_->Init();
                RS_RETURN_ON_NEQ(ret, RS_SUCCESS, "node init failure\n");
                iter->node_->SetInitStatus(true);
            }

            edge_repository_ = edge_repository;

            return ret;
        }

        ErrorCode ParallelTaskEngine::DeInit() {
            ErrorCode ret = RS_SUCCESS;
            thread_pool_->DeInit();
            delete thread_pool_;

            for (auto iter : topo_sort_node_) {
                ret = iter->node_->Deinit();
                RS_RETURN_ON_NEQ(ret, RS_SUCCESS, "node deinit failed\n");
                iter->node_->SetInitStatus(false);
            }
            return ret;
        }

        ErrorCode ParallelTaskEngine::Setup() {
            return RS_SUCCESS;
        }

        ErrorCode ParallelTaskEngine::Run() {
            ErrorCode ret = RS_SUCCESS;

            for (const auto &node : start_nodes_) {
                Process(node);
            }
            Wait();

            //  check no running node
            for (const auto &node : topo_sort_node_) {
                if (node->color_ != NODE_COLOR_BLACK) {
                    std::string err_msg = "node " + node->name_ + " is not finish!";
                    RS_LOGE("%s\n", err_msg.c_str());
                    return RS_NODE_STATU_ERROR;
                }
            }

            AfterGraphRun();

            return ret;
        }

        bool ParallelTaskEngine::Synchronize() {
            // for (auto iter : topo_sort_node_) {
            //     if (iter->node_->Synchronize() == false) {
            //         return false;
            //     }
            // }
            return true;
        }

        // global_status_ is refer from Cgraph.
        void ParallelTaskEngine::Process(NodeWrapper *node_wrapper) {
            if (unlikely(global_status_ != RS_SUCCESS)) {
                RS_LOGW("global status is not success\n");
                return;
            }

            node_wrapper->color_ = NODE_COLOR_GRAY;

            const auto &func = [this, node_wrapper] {
                ErrorCode cur_ret = node_wrapper->node_->Run();
                if (unlikely(cur_ret != RS_SUCCESS)) {
                    RS_LOGE("[%s] run error: %d\n", node_wrapper->node_->GetName().c_str(),
                            cur_ret);

                    std::lock_guard<std::mutex> status_lock(status_lock_);

                    if (global_status_ == RS_SUCCESS) {
                        global_status_ = cur_ret;
                    } else {
                        // global_status_ is not success, no longer assign value.
                        RS_LOGI("global status is [%d],cur status is [%d].\n", global_status_,
                                cur_ret);
                    }
                }
                AfterNodeRun(node_wrapper);
            };

            thread_pool_->Commit(func);
        }
        // Recursion,from top to bottom.
        void ParallelTaskEngine::AfterNodeRun(NodeWrapper *node_wrapper) {
            {
                std::lock_guard<std::mutex> lock(main_lock_);
                completed_task_count_++;
            }
            // node status refresh.
            node_wrapper->color_ = NODE_COLOR_BLACK;
            if (!node_wrapper->successors_.empty() && (global_status_ == RS_SUCCESS)) {
                for (auto successor : node_wrapper->successors_) {
                    bool all_pre_done = true;
                    for (auto iter : successor->predecessors_) {
                        all_pre_done &= (iter->color_ == NODE_COLOR_BLACK);
                    }
                    if (all_pre_done && successor->color_ == NODE_COLOR_WHITE) {
                        if (successor->predecessors_.size() <= 1) {
                            Process(successor);
                        } else {
                            SubmitTaskSynchronized(successor);
                        }
                    }
                }
            } else {
                std::lock_guard<std::mutex> lock(main_lock_);
                // completed_task_count_++;
                /**
                 * 满足一下条件之一，则通知wait函数停止等待
                 * 1，无后缀节点全部执行完毕(在运行正常的情况下，只有无后缀节点执行完成的时候，才可能整体运行结束)
                 * 2，有节点执行状态异常
                 */

                if ((node_wrapper->successors_.empty()
                     && (completed_task_count_ >= all_task_count_))
                    || (global_status_ != RS_SUCCESS)) {
                    cv_.notify_one();
                }
            }
        }

        // clear running status.
        void ParallelTaskEngine::AfterGraphRun() {
            completed_task_count_ = 0;
            for (auto &node : topo_sort_node_) {
                node->color_ = NODE_COLOR_WHITE;
                // reset node color to white(no runing) ready for next run
            }
        }

        void ParallelTaskEngine::SubmitTaskSynchronized(NodeWrapper *node_wrapper) {
            std::lock_guard<std::mutex> lock(commit_lock_);
            if (node_wrapper->color_ == NODE_COLOR_WHITE) {
                Process(node_wrapper);
            }
        }
        void ParallelTaskEngine::Wait() {
            std::unique_lock<std::mutex> lock(main_lock_);
            /**
             * 主线程等待检查；
             * 遇到以下条件之一，结束执行：
             * 1，执行结束
             * 2，状态异常
             */

            cv_.wait(lock, [this] {
                return ((completed_task_count_ >= all_task_count_)
                        || (global_status_ != RS_SUCCESS));
            });
        }

    } // namespace dag
} // namespace rayshape