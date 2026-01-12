#ifndef _LOCAL_THREAD_H_
#define _LOCAL_THREAD_H_

#include <memory>
#include <thread>

#include "thread_pool/safe_ws_queue.h"
#include "thread_pool/runnable_task.h"

/*
    local thread
*/

namespace rayshape
{
    namespace threadpool
    {
        class LocalThread {
        public:
            explicit LocalThread() {
                done_ = true;
                pool_threads_ = nullptr;
                index_ = -1;
                total_thread_size_ = 0;
            }
            ~LocalThread() {
                DeInit();
            }

            void DeInit() {
                done_ = false;
                if (thread_.joinable()) {
                    thread_.join(); // wait for thread to finish
                }
            }

            void Init() {
                done_ = true;
                BuildStealTargets();
                thread_ = std::move(std::thread(&LocalThread::Run, this));
            }

            void SetThreadPoolInfo(int index, std::vector<LocalThread *> *pool_threads) {
                index_ = index;
                pool_threads_ = pool_threads;
                total_thread_size_ = (int)pool_threads->size();
            }

            /**
             * 线程执行函数
             * @return
             */
            ErrorCode Run() {
                if (std::any_of(pool_threads_->begin(), pool_threads_->end(),
                                [](LocalThread *thd) { return nullptr == thd; })) {
                    return RS_THREAD_POLL_ERROR;
                }

                while (done_) {
                    RTask task;
                    if (PopTask(task) || StealTask(task)) {
                        task();
                    } else {
                        std::unique_lock<std::mutex> lk(mutex_);
                        cv_.wait_for(lk, std::chrono::milliseconds(100));
                    }
                }
                return RS_SUCCESS;
            }

            /**
             * 依次push到任一队列里。如果都失败，则yield，然后重新push
             * @param task
             * @return
             */
            void PushTask(RTask &&task) {
                while (!(primary_queue_.TryPush(std::forward<RTask>(task)))) {
                    std::this_thread::yield();
                }
                cv_.notify_one();
            }

            /**
             * 从本地弹出一个任务
             * @param task
             * @return
             */
            bool PopTask(RTask &task) {
                return primary_queue_.TryPop(task);
            }

            /**
             * 从其他线程窃取一个任务
             * @param task
             * @return
             */
            bool StealTask(RTask &task) {
                if (pool_threads_->size() < total_thread_size_) {
                    /**
                     * 线程池还未初始化完毕的时候，无法进行steal。
                     * 确保程序安全运行。
                     */

                    return false;
                }

                for (auto &target : steal_targets_) {
                    /**
                     * 从线程中周围的thread中，窃取任务。
                     * 如果成功，则返回true，并且执行任务。
                     * steal 的时候，先从第二个队列里偷，从而降低触碰锁的概率
                     */

                    if (((*pool_threads_)[target])
                        || ((*pool_threads_)[target])->primary_queue_.TrySteal(task)) {
                        return true;
                    }
                }

                return false;
            }

        private:
            // 生成各个局部线程如何从其他线程窃取目标的策略.
            void BuildStealTargets() {
                steal_targets_.clear();
                for (int i = 0; i < total_thread_size_ - 1; i++) {
                    auto target = (index_ + i + 1) % total_thread_size_;
                    steal_targets_.push_back(target);
                }
                steal_targets_.shrink_to_fit();
            }

        private:
            bool done_;          // 线程状态标记
            std::thread thread_; // local thread

            unsigned int index_; // 线程索引
            int total_thread_size_;

            SafeWSQueue<RTask> primary_queue_;         // 任务队列
            std::vector<LocalThread *> *pool_threads_; // 全局线程池
            std::vector<int> steal_targets_;           // 从其他线程窃取任务，存储的是其他线程的索引
            std::mutex mutex_;
            std::condition_variable cv_;
        };

    } // namespace threadpool
} // namespace rayshape

#endif
