#ifndef _DAG_EDGE_DATA_PACKAGE_H_
#define _DAG_EDGE_DATA_PACKAGE_H_

#include "base/common.h"
#include "base/error.h"
#include "device/abstract_device.h"

#ifdef ENABLE_3RD_OPENCV
#include <opencv2/opencv.hpp>
#endif

// create blob管道数据来进行数据传递

namespace rayshape
{
    namespace dag
    { // 1.数据包的数据最好用结构体类这样的数据进行传递和填充不适合c++自带的数据类型
        // 2.可以开辟针对自有的数据类型进行设置比如buffer,blob类型的数据可以创建特定API
        /**
         * @brief
         * 数据包的状态
         * # 未创建
         * # 已创建
         * # 已写入
         * # 没被消费
         * # 正在被消费
         * # 被消费过
         * # 被所有节点消费
         */
        // data package may be is a struct contains complex data.
        class DataPackage: public NonCopyable {
        public:
            DataPackage();
            virtual ~DataPackage();

            virtual ErrorCode SetBuff(Buffer *buffer, bool is_external);
            Buffer *CreateBuff(device::AbstractDevice *device,
                               RSMemoryInfo mem_info); // 一整块连续内存
            virtual bool NotifyWrite(Buffer *buffer);
            virtual Buffer *GetBuff();
#ifdef ENABLE_3RD_OPENCV
            //
            virtual ErrorCode SetMat(cv::Mat *mat, bool is_external);

            cv::Mat *CreateMat(int rows, int cols, int type, const cv::Scalar &value);

            virtual bool NotifyWrite(cv::Mat *mat);

            virtual cv::Mat *GetMat();

#endif

            // buffer数据contrl
            /**
             * @brief Set any data to data package.
             */
            template <typename T>
            ErrorCode SetAny(T *t, bool is_external);

            /**
             * @brief Create any data to data package.
             */
            template <typename T, typename... Args>
            T *CreateAny(int index, Args &&...args);

            template <typename T>
            T *GetAny(); // set anything 数据包

            void SetIndex(int64_t index);
            int64_t GetIndex();

        protected:
            /**
             * @brief destory inner create any data package.
             */
            void Destory();

        protected:
            int64_t index_ = -1;       // package index 在pipeline的边类型中,需要控制流水线运行顺序
            bool is_external_ = false; // 是否外部数据包,set 为外部,

            bool written_ = false; // 数据包是否写入

            /* 为啥用指针？优点：内存复用,缺点：内存的释放把控*/
            void *data_anything_ = nullptr;            // 存放任意数据
            std::function<void(void *)> data_deleter_; // 析构函数
            std::type_info *type_info_;                // 模板编程，记录数据类型
        };

        // pipeline package. 一般就两种数据类型：
        class PipelineDataPackage: public DataPackage {
        public:
            PipelineDataPackage(int consumers_size);

            virtual ~PipelineDataPackage();

            virtual ErrorCode SetBuff(Buffer *buffer, bool is_external);
            virtual bool NotifyWrite(Buffer *buffer);
            virtual Buffer *GetBuff();
            Buffer *GetBufferDirect();
#ifdef ENABLE_3RD_OPENCV
            virtual ErrorCode SetMat(cv::Mat *mat, bool is_external);

            virtual bool NotifyWrite(cv::Mat *mat);

            virtual cv::Mat *GetMat();

            cv::Mat *GetMatDirect();
#endif
            virtual ErrorCode TakeDataPackage(DataPackage *package);

            void IncreaseConsumersSize();
            void IncreaseConsumersCount();

            int GetConsumersSize();
            int GetConsumersCount();

        protected:
            std::mutex mutex_;
            std::condition_variable cv_;
            int consumers_size_ = 0;  // pipeline 边所持有的特定的pipeline数据包的个数
            int consumers_count_ = 0; //
        };

    } // namespace dag

} // namespace rayshape

#endif
