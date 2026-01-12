#include "dag/engine/parallel_pipeline_engine.h"
#include "dag/util.h"

namespace rayshape
{
    namespace dag
    {
        ParallelPipelineEngine::ParallelPipelineEngine() : ExecuteEngine() {
            thread_pool_ = nullptr;
            run_size_ = 0;
            completed_size_ = 0;
        }

        ParallelPipelineEngine::~ParallelPipelineEngine() {}

        ErrorCode ParallelPipelineEngine::Init(std::vector<EdgeWrapper *> &edge_repository,
                                               std::vector<NodeWrapper *> &node_repository) {
            ErrorCode ret = TopoSortDFS(node_repository, topo_sort_node_);
            // 前 infer 后处理三个节点
            for (auto iter : topo_sort_node_) {
                iter->color_ = NODE_COLOR_WHITE;
                if (iter->node_->GetInitStatus()) {
                    continue; // check node init statu,if node not init will
                }

                ret = iter->node_->Init();
                RS_RETURN_ON_NEQ(ret, RS_SUCCESS, "node init failure\n");
                iter->node_->SetInitStatus(true);
            }
            all_task_count_ = static_cast<int>(topo_sort_node_.size());
            edge_repository_ = edge_repository;

            thread_pool_ = new threadpool::ThreadPool(all_task_count_);
            ret = thread_pool_->Init();
            RS_RETURN_ON_NEQ(ret, RS_SUCCESS, "ThreadPool init failure\n");
            // 每个线程塞一个节点任务执行.
            this->CommitThreadPool();
            return RS_SUCCESS;
        }

        ErrorCode ParallelPipelineEngine::DeInit() {
            ErrorCode ret = RS_SUCCESS;
            if (!is_synchronize_) {
                this->Synchronize();
            }

            for (auto iter : edge_repository_) {
                bool flag = iter->edge_->RequestTerminate();
                if (!flag) {
                    RS_LOGE("failed iter edge requestTerminate()!\n");
                    return RS_UNKNOWN;
                }
            }
            ret = thread_pool_->DeInit();
            RS_RETURN_ON_NEQ(ret, RS_SUCCESS, "thread_pool_ DeInit failure\n");
            delete thread_pool_;

            for (auto iter : topo_sort_node_) {
                ret = iter->node_->Deinit();
                RS_RETURN_ON_NEQ(ret, RS_SUCCESS, "node deinit failed\n");
                iter->node_->SetInitStatus(false);
            }

            return ret;
        }
        ErrorCode ParallelPipelineEngine::Setup() {
            return RS_SUCCESS;
        }

        ErrorCode ParallelPipelineEngine::Run() {
            run_size_++;
            return RS_SUCCESS;
        }

        bool ParallelPipelineEngine::Synchronize() {
            std::unique_lock<std::mutex> lock(mutex_);
            // 使用条件变量等待所有节点完成执行.
            // Lambda函数检查所有节点是否完成当前轮次的运行.
            pipeline_cv_.wait(lock, [this]() {
                bool flag = false;
                for (auto iter : topo_sort_node_) {
                    // 检查节点是否达到预期的运行数.
                    if (iter->node_->GetRunCompletedSize() < run_size_) {
                        flag = false;
                        break;
                    }
                    completed_size_ = run_size_;
                    flag = true;
                }
                return flag; // 有任何节点未完成，则继续等待
            });
            // 考虑删除,感觉是无用的代码
            for (auto iter : topo_sort_node_) {
                if (iter->node_->Synchronize() == false) {
                    return false;
                }
            }
            // 更新运行状态
            is_synchronize_ = completed_size_ == run_size_;

            return is_synchronize_;
        }

        void ParallelPipelineEngine::CommitThreadPool() {
            // 对于topo排序的每个节点
            // 并行执行：每个节点作为一个独立任务提交到线程池，实现真正的并行处理
            // 流水线模式：通过输入边的状态驱动节点执行，形成流水线效果

            for (auto iter : topo_sort_node_) {
                auto func = [iter, this]() -> ErrorCode {
                    ErrorCode ret_status = RS_SUCCESS;

                    while (true) {
                        // 检查输入边的状态
                        // 这里应该是要依赖节点运行完成后进行数据传递时更新.
                        EdgeUpdateFlag edge_update_flag = iter->node_->UpdateInput();
                        if (edge_update_flag == EdgeUpdateFlag::Complete) {
                            iter->node_->SetRunningFlag(true);
                            ret_status = iter->node_->Run();
                            RS_RETURN_ON_NEQ(ret_status, RS_SUCCESS, "node execute failed!\n");
                            iter->node_->SetRunningFlag(false);

                            if (iter->node_->GetRunCompletedSize() == run_size_) {
                                pipeline_cv_.notify_all();
                            }
                        } else if (edge_update_flag == EdgeUpdateFlag::Terminate) {
                            RS_LOGI("node [%s] UpdateInput terminate!\n",
                                    iter->node_->GetName().c_str());
                            break;
                        } else {
                            RS_LOGE("failed to node [%s] UpdateInput()!\n",
                                    iter->node_->GetName().c_str());
                            ret_status = RS_UNKNOWN;
                            break;
                        }
                    }
                    return ret_status;
                };
                thread_pool_->Commit(std::bind(func));
            }
        }

    } // namespace dag
} // namespace rayshape