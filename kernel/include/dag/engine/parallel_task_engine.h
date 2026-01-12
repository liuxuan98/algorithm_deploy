#ifndef _PARALLEL_TASK_ENGINE_H_
#define _PARALLEL_TASK_ENGINE_H_

#include "dag/engine.h"
#include "thread_pool/thread_pool.h"

/*
//noly finish dynamic engine of cgraph.
针对的是非串行序列和全并行的图的运行场景
*/

namespace rayshape
{
    namespace dag
    {
        class ParallelTaskEngine: public ExecuteEngine {
        public:
            ParallelTaskEngine();
            virtual ~ParallelTaskEngine();

            virtual ErrorCode Init(std::vector<EdgeWrapper *> &edge_repository,
                                   std::vector<NodeWrapper *> &node_repository);

            virtual ErrorCode Setup();

            virtual ErrorCode DeInit();

            virtual ErrorCode Run();

        private:
            /**
             * @brief commit a node to execute.
             * @param  node_wrapper
             */
            void Process(NodeWrapper *node_wrapper);

            /**
             * @brief node process after do something; node status refresh, add successors node
             * wake up main thread
             *
             * @param  node_wrapper
             */
            void AfterNodeRun(NodeWrapper *node_wrapper);

            /**
             * node running wait
             * @param
             * @return
             */
            void Wait();

            /**
             * 用于清空节点运行状态 and 全局任务完成数量
             * @param
             * @return
             */
            void AfterGraphRun();

            /**
             * @brief  一个节点有多个前驱节点时,多线程异步任务情况下,防止多次加入执行
             * @param  node_wrapper
             */
            void SubmitTaskSynchronized(NodeWrapper *node_wrapper);

        private:
            // 调度引擎持有线程池
            threadpool::ThreadPool *thread_pool_ = nullptr;  // 线程池,聚合关系
            std::vector<NodeWrapper *> topo_sort_node_;      // 拓扑排序后的所有节点
            std::vector<NodeWrapper *> start_nodes_;         // 没有依赖的起始节点

            std::atomic<int> completed_task_count_{0}; // 已执行结束的元素个数
            int all_task_count_ = 0;                   // 需要执行的所有节点个数

            std::mutex main_lock_;
            std::mutex commit_lock_;
            std::condition_variable cv_;
            std::mutex status_lock_;
            ErrorCode global_status_ = RS_SUCCESS; // 全局任务的状态，改状态时

            std::vector<EdgeWrapper *> edge_repository_; // 边的仓库
        };

    } // namespace dag
} // namespace rayshape

#endif