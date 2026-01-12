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
            // how decide the thread number? 最大的层序遍历节点数量
            /**
             * 1. 创建线程池
             * 2. 判断是否是 dag 结构(在拓扑排序里面做了)
             * 3. 标记数据,比如task node总量,起始节点数量等
             * 4. 初始化节点,比如节点的状态
             * 5. 可以做节点结构类型(连接后继节点的情形)的分析,如root node,link node, normal
             */
            thread_pool_ = new threadpool::ThreadPool(); // default thread number is 4.
            thread_pool_->Init();

            start_nodes_ = FindStartNodes(node_repository);
            if (start_nodes_.empty()) {
                RS_LOGE("No start node found in graph.\n");
                return RS_INVALID_PARAM_VALUE;
            }
            ret = TopoSortBFS(node_repository, topo_sort_node_); // 是否有向无环图
            if (ret != RS_SUCCESS) {
                RS_LOGE("TopoSortBFS Failed!\n");
                return ret;
            }
            all_task_count_ = static_cast<int>(topo_sort_node_.size()); // how to work?

            for (auto iter : topo_sort_node_) {
                // node init
                iter->color_ = NODE_COLOR_WHITE;
                if (iter->node_->GetInitStatus()) {
                    continue; // check node init statu,if node not init will
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
            // edge terminate
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
            Wait(); // wait all task finish,会一致阻塞，直到所有任务结束
            // 主线程一致会卡死的
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
        // global_status_ 是参考Cgraph中的
        void ParallelTaskEngine::Process(NodeWrapper *node_wrapper) {
            if (unlikely(global_status_ != RS_SUCCESS)) {
                RS_LOGW("global status is not success\n"); // debug可以add thread id.
                return;
            }

            node_wrapper->color_ = NODE_COLOR_GRAY;

            const auto &func = [this, node_wrapper] {
                // 节点
                ErrorCode cur_ret = node_wrapper->node_->Run();
                if (unlikely(cur_ret != RS_SUCCESS)) {
                    // 当且仅当整体状正常，且当前状态异常的时候，进入赋值逻辑。确保不重复赋值
                    RS_LOGE("[%s] run error: %d\n", node_wrapper->node_->GetName().c_str(),
                            cur_ret);

                    std::lock_guard<std::mutex> status_lock(status_lock_);
                    // 如果
                    if (global_status_ == RS_SUCCESS) {
                        global_status_ = cur_ret;
                    } else {
                        // 全局状态已经不正常，则不再赋值
                        RS_LOGI("global status is [%d],cur status is [%d].\n", global_status_,
                                cur_ret);
                    }

                    // return ret; // 直接报错返回,no think about global error
                    //  status.任务会卡在该节点不会进入后继节点，后续任务不会递交到任务队列
                }
                AfterNodeRun(node_wrapper);
            };

            // 此处线程亲和性概念 把任务直接丢入当前线程运行.
            thread_pool_->Commit(func);
        }
        // 递归,自顶向下
        void ParallelTaskEngine::AfterNodeRun(NodeWrapper *node_wrapper) {
            {
                std::lock_guard<std::mutex> lock(main_lock_);
                completed_task_count_++;
            }
            node_wrapper->color_ = NODE_COLOR_BLACK; // node status refresh.
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
                    cv_.notify_one(); // 唤醒engine主线程
                }
                // finish node/task
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
        // 该节点由多个前驱节点，节点都是异步执行的，
        // 且多个前驱节点都执行完毕的时候，该节点才会提交
        void ParallelTaskEngine::SubmitTaskSynchronized(NodeWrapper *node_wrapper) {
            std::lock_guard<std::mutex> lock(commit_lock_);
            if (node_wrapper->color_ == NODE_COLOR_WHITE) {
                Process(node_wrapper);
            }
        }
        // 节点运行状态
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