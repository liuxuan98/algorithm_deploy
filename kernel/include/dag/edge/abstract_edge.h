#ifndef _DAG_EDGE_ABSTRACT_EDGE_H_
#define _DAG_EDGE_ABSTRACT_EDGE_H_

#include "base/error.h"
#include "base/common.h"
// #include "dag/node.h"
#include "data_package.h"

namespace rayshape
{
    namespace dag
    {

        class Node;
        /**
         * @brief 抽象边类,在此基础上扩展fixededge and pipeline_edge
         * @details 边管理生产消费者节点的关系
         */

        class AbstractEdge: public NonCopyable {
        public:
            AbstractEdge(ParallelType paralle_type);
            virtual ~AbstractEdge();

            // virtual ErrorCode Create(); // no need
            //  获得边的并行类型
            ParallelType GetParallelType();

            // 对边设置最大队列大小,对流水线包有用,// 这个队列接口似乎只对piep
            virtual ErrorCode SetQueueMaxSize(int queue_max_size) = 0;

            /**
             * 边的数据包管理
             */
            virtual ErrorCode Construct() = 0;

            virtual ErrorCode SetBuff(Buffer *buffer, bool is_external) = 0;
            virtual Buffer *CreateBuff() = 0;             // 看是否需要更改
            virtual bool NotifyWrite(Buffer *buffer) = 0; // 唤醒
            // virtual Buffer *GetBuff() = 0;                // 获得数

            virtual Buffer *GetBuff(const Node *node) = 0; // 获得数据包消费

            virtual Buffer *GetGraphOutputBuffer() {
                return nullptr;
            }
#ifdef ENABLE_3RD_OPENCV
            virtual ErrorCode SetMat(cv::Mat *mat, bool is_external) = 0;
            virtual cv::Mat *CreateMat(int rows, int cols, int type, const cv::Scalar &value) = 0;
            virtual bool NotifyWrite(cv::Mat *mat) = 0;
            virtual cv::Mat *GetMat(const Node *node) = 0;

            virtual cv::Mat *GetGraphOutputMat() {
                return nullptr;
            }

#endif
            // 设置其他类型的数据
            template <typename T>
            ErrorCode SetAny(T *t, bool is_external);

            /**
             * @brief set ,create notify,get 等数据处理的接口
             *
             */

            // 内部创建
            // get 得到包中的数据指针.
            template <typename T>
            T *GetAny(const Node *node);

            // 从不同类型的边中拿数据包,这各接口是否需要？？
            virtual ErrorCode TakeDataPackage(DataPackage *data_pack) = 0;

            virtual EdgeUpdateFlag Update(const Node *node) = 0;

            // 管理生产和消费者
            std::vector<Node *> GetProducers();
            ErrorCode IncreaseProducers(std::vector<Node *> &producers);

            std::vector<Node *> GetConsumers();
            ErrorCode IncreaseConsumers(std::vector<Node *> &consumers);

            virtual bool RequestTerminate() = 0;

        protected:
            void IncreaseIndex();

            bool CheckNode(const Node *node);

        protected:
            int64_t index_ = -1; // 边中的数据包索引

            ParallelType parallel_type_;
            /**
             * @brief
             * 按理来说，一个edge应该只被一个生产者一个消费者使用，但是为了兼容性，这里使用vector
             */
            std::vector<Node *> producers_; // 边子类可访问的生产者节点
            std::vector<Node *> consumers_; // 边子类可访问的消费者节点

            bool terminate_flag_ = false; // 边是否终止,

            DataPackage *data_pack_ = nullptr; // 边的数据包可以是任意类型的数据
        };

        /**
         *工厂模式的设计模式,create different type edge.
         *
         *
         */

        class EdgeCreator {
        public:
            virtual ~EdgeCreator() {};
            virtual AbstractEdge *CreateEdge(ParallelType paralle_type) = 0;
        };

        template <typename T>
        class TypeEdgeCreator: public EdgeCreator {
            virtual AbstractEdge *CreateEdge(ParallelType paralle_type) override {
                return new T(paralle_type);
            }
        };

        std::map<EdgeType, std::shared_ptr<EdgeCreator>> &GetGlobalEdgeCreatorMap();

        template <typename T>
        class TypeEdgeRegister {
        public:
            explicit TypeEdgeRegister(EdgeType type) {
                GetGlobalEdgeCreatorMap()[type] = std::shared_ptr<T>(new T());
                // std::make_shared<TypeEdgeCreator<T>>();
            }
        };

        AbstractEdge *CreateEdge(ParallelType paralle_type);

        AbstractEdge *RecreateEdge(AbstractEdge *abstact_edge, const ParallelType &paralle_type);
    } // namespace dag
} // namespace rayshape

#endif // !_DAG_EDGE_ABSTRACT_EDGE_H_