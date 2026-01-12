#ifndef _PARALLEL_PIPELINE_ENGINE_H_
#define _PARALLEL_PIPELINE_ENGINE_H_

#include "dag/engine.h"
#include "thread_pool/thread_pool.h"
/**
 * @brief 流水线DAG执行引擎.参考nnde.ai流水线执行引擎
 */

namespace rayshape
{
    namespace dag
    {
        class ParallelPipelineEngine: public ExecuteEngine {
        public:
            ParallelPipelineEngine();

            virtual ~ParallelPipelineEngine();

            virtual ErrorCode Init(std::vector<EdgeWrapper *> &edge_repository,
                                   std::vector<NodeWrapper *> &node_repository);

            virtual ErrorCode DeInit();

            virtual ErrorCode Run();

            virtual ErrorCode Setup();
            // 注释掉

            bool Synchronize();

        protected:
            void CommitThreadPool();

        private:
            // 调度引擎持有线程池
            threadpool::ThreadPool *thread_pool_ = nullptr; // 线程池,聚合关系
            std::vector<NodeWrapper *> topo_sort_node_;     // 拓扑排序后的所有节点

            int all_task_count_ = 0;

            std::mutex mutex_;
            std::condition_variable pipeline_cv_;

            std::vector<EdgeWrapper *> edge_repository_; // 边的仓库

            /**
             * @brief 当前提交到流水线的任务数量
             *
             * 记录已提交但尚未完全处理完的任务总数，即需要运行的总数
             */
            size_t run_size_ = 0;

            /**
             * @brief 已完成处理的任务数量
             *
             * 记录已经完成处理的任务数，用于跟踪流水线进度和同步
             */
            size_t completed_size_ = 0;

            bool is_synchronize_ = false; // 是否同步
            // 需要有锁配合变量改动
            // ErrorCode global_status_ = RS_SUCCESS; // 全局任务的状态，改状态时
        };
    } // namespace dag
} // namespace rayshape

#endif