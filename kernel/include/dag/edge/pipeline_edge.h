#ifndef DAG_PIPELINE_EDGE_H
#define DAG_PIPELINE_EDGE_H

#include "abstract_edge.h"

namespace rayshape
{
    namespace dag
    {

        class PipelineEdge: public AbstractEdge {
        public:
            PipelineEdge(ParallelType paralle_type);

            virtual ~PipelineEdge();

            virtual ErrorCode SetQueueMaxSize(int queue_max_size) override;
            /*边的构建*/
            virtual ErrorCode Construct() override;

            virtual ErrorCode SetBuff(Buffer *buffer, bool is_external) override;
            virtual Buffer *CreateBuff() override;
            virtual bool NotifyWrite(Buffer *buffer) override;
            virtual Buffer *GetBuff(const Node *node) override;
            // 需要添加最后一条边获取结果的接口getGraphOutputBuffer，这条边是没有消费者的边 #TODO
            virtual Buffer *GetGraphOutputBuffer() override;
#ifdef ENABLE_3RD_OPENCV
            virtual ErrorCode SetMat(cv::Mat *mat, bool is_external) override;
            virtual cv::Mat *CreateMat(int rows, int cols, int type,
                                       const cv::Scalar &value) override;
            virtual bool NotifyWrite(cv::Mat *mat) override;
            virtual cv::Mat *GetMat(const Node *node) override; // 从具体的节点中获取Mat
                                                                //

            virtual cv::Mat *GetGraphOutputMat() override;
#endif

            virtual ErrorCode TakeDataPackage(DataPackage *data_pack) override;

            virtual EdgeUpdateFlag Update(const Node *node) override;

            virtual bool RequestTerminate() override;

        private:
            PipelineDataPackage *GetPipelineDataPacket(const Node *node);

        private:
            std::mutex mutex_;
            std::condition_variable cv_;
            // 边消费者数量,pipeline模式下边消费者数量为一般为1
            int consumers_size_;

            std::condition_variable queue_cv_;
            int queue_max_size_;
            // 数据包
            // 数据包列表
            std::list<PipelineDataPackage *> data_packets_;
            /**
             * 允许数据包任意位置快速插入和删除元素(不需要重新分配内存),支持双向迭代器.
             */

            // 每个消费者节点 消费 的数据包最新索引  与下面当前数据包的关系为该索引为其+1
            std::map<Node *, int> to_consume_index_;

            // 每个消费者 消费 的当前数据包
            std::map<Node *, PipelineDataPackage *> consuming_dp_;
        };

    } // namespace dag
} // namespace rayshape

#endif