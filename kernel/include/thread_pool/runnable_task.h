#ifndef _RUNNABLE_TASK_H_
#define _RUNNABLE_TASK_H_

#include <functional>
#include <memory>

namespace rayshape
{
    namespace threadpool
    {
        // 通用任务封装类,封装(function\lambda\)统一任务接口
        class RunnableTask {
            struct TaskBased {
                explicit TaskBased() = default;
                virtual void call() = 0;
                virtual ~TaskBased() = default;
            };

            template <typename F, typename T = typename std::decay<F>::type>
            struct TaskDerived: TaskBased {
                T func_;
                explicit TaskDerived(F &&func) : func_(std::forward<F>(func)) {}
                void call() override {
                    func_();
                }
            };
            // 使用typename T = typename std::decay<F>::type获取任务函数的真实返回类型
        public:
            // Keep the original templated constructor for lambdas and other callable
            // objects
            template <typename F>
            RunnableTask(F &&f) : impl_(new TaskDerived<F>(std::forward<F>(f))) {}

            void operator()() {
                if (impl_) {
                    impl_->call();
                }
            }

            RunnableTask() = default;

            RunnableTask(RunnableTask &&task) noexcept : impl_(std::move(task.impl_)) {}

            RunnableTask &operator=(RunnableTask &&task) noexcept {
                impl_ = std::move(task.impl_);
                return *this;
            }
            RunnableTask(const RunnableTask &) = delete;
            RunnableTask &operator=(const RunnableTask &) = delete;

        private:
            std::unique_ptr<TaskBased> impl_;
        };

        using RTask = RunnableTask;

    } // namespace threadpool
} // namespace rayshape

#endif
