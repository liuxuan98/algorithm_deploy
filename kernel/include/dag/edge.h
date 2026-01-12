#ifndef DAG_EDGE_H
#define DAG_EDGE_H

#include "base/error.h"
#include "base/common.h"
#include "edge/abstract_edge.h"
#include "base/glic_stl_include.h"
#ifdef ENABLE_3RD_OPENCV
#include <opencv2/opencv.hpp>
#endif

// 两节点数据装载的容器 name as edge.

// 使用桥接的设计模式
namespace rayshape
{
    namespace dag
    {
        /**
         * 桥接设计模式,抽象边基础上继承有固定边即DAG_EDGE,pipeline变等的行为
         * 边管理和支持的数据有buffer,cv::Mat目前的两种基本的数据类型
         * 边要支持内存管理,还要支持特定的数据结构
         * pipeline的边才涉及到队列数据的索引管理
         * /

        /**
        1.生产节点和消费节点中间扮演连接两者的角色，持有的中间数据,可以是任何数据.
        2.一条边就只能连接两个节点，不能连接多个节点.
        */

        class RS_PUBLIC Edge: public NonCopyable {
        public:
            Edge();

            Edge(const std::string &name);

            virtual ~Edge();

            std::string GetName() const;

            /**
             * @brief Set the Parallel Type object

              * @note 在construct之前，调用该函数，内部创建出具体类型的edge,来传导数据
             */

            ErrorCode SetParallelType(const ParallelType &paralle_type);

            ParallelType GetParallelType();
            /**
             * @brief 边的构建
             */
            ErrorCode Construct();

            /**
             * @brief buffer 数据相关
             *
             */
            ErrorCode SetBuff(Buffer *buffer, bool is_external = true);

            Buffer *CreateBuffer();

            bool NotifyWrite(Buffer *buffer);

            Buffer *GetBuff(const Node *node = nullptr);

            Buffer *GetGraphOutputBuffer();
#ifdef ENABLE_3RD_OPENCV
            ErrorCode SetMat(cv::Mat *mat, bool is_external = true);
            cv::Mat *CreateMat(int rows, int cols, int type, const cv::Scalar &value);

            bool NotifyWrite(cv::Mat *mat);
            cv::Mat *GetMat(const Node *node = nullptr);

            cv::Mat *GetGraphOutputMat();
#endif
            EdgeUpdateFlag Update(const Node *node); // 边更新输出给消费者

            bool RequestTerminate();

            /**
            @brief 给边添加生产消费节点
            */
            ErrorCode IncreaseProducers(std::vector<Node *> &producers);

            ErrorCode IncreaseConsumers(std::vector<Node *> &consumers);

            /**
             * @brief 在边中设置外部数据任意类型的指针
             */
            template <typename T>
            ErrorCode SetData(T *t) {
                // 设置类型信息;
                return abstract_edge_->SetData<T>(t, true);
            }

            /**
             * @brief 唤醒写数据
             */
            template <typename T>
            bool NotifyWrite(T *t) {
                return abstract_edge_->NotifyWrite<T>(t);
            }

            /**
             * @brief get node 从节点获取输出数据边中的数据？
             *
             */
            template <typename T>
            T *GetData(const Node *node) {
                return abstract_edge_->GetData<T>(node);
            }

        private:
            std::string name_; // edge name
            AbstractEdge *abstract_edge_ = nullptr;
            /* brige connect mode */
        };

    } // namespace dag
} // namespace rayshape

#endif