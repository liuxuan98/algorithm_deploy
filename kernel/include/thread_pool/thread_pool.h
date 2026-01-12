#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_

// 线程池 管理线程类
#include <atomic>
#include <future>
#include <thread>

#include "base/error.h"
#include "thread_pool/local_thread.h"

// #include <thread>

namespace rayshape
{
    namespace threadpool
    {
        class ThreadPool {
        public:
            explicit ThreadPool(unsigned int size = 4) {
                max_thread_size_ = size;
            };

            ~ThreadPool() {};

            ErrorCode Init() {
                // 具备当前线程能看到之前的局部线程对象池的能力,比如第1个能看到包含0的线程对象，第二个能看到包含1和0的线程对象
                for (int i = 0; i < max_thread_size_; i++) {
                    LocalThread *ptr = new LocalThread();
                    ptr->SetThreadPoolInfo(i, &threads_);
                    threads_.push_back(ptr);
                }
                for (int i = 0; i < max_thread_size_; i++) {
                    threads_[i]->Init();
                    // 初始化线程
                }
                return RS_SUCCESS;
            }

            ErrorCode DeInit() {
                for (auto &ptr_thread : threads_) {
                    ptr_thread->DeInit();
                    delete ptr_thread;
                }
                return RS_SUCCESS;
            }

            // void run();
            template <typename FunctionType>
            auto Commit(const FunctionType &func)
                -> std::future<decltype(std::declval<FunctionType>()())> {
                using ResultType = decltype(std::declval<FunctionType>()());
                std::packaged_task<ResultType()> task(func);
                std::future<ResultType> result(task.get_future());
                // 均匀提交策略 和Cgraph有差异
                cur_index_++;
                if (cur_index_ >= max_thread_size_ || cur_index_ < 0) {
                    cur_index_ = 0;
                }
                threads_[cur_index_]->PushTask(std::move(task)); // 右值
                return result;
            }

        private:
            unsigned int max_thread_size_ = 0;
            std::atomic<int> cur_index_{0};
            std::vector<LocalThread *> threads_; // 主线程任务primary_threads_
        };

    } // namespace threadpool
} // namespace rayshape

#endif
