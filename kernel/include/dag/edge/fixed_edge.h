#ifndef DAG_FIXED_EDGE_H
#define DAG_FIXED_EDGE_H

#include "abstract_edge.h"
// #

namespace rayshape
{
    namespace dag
    {
        class FixedEdge: public AbstractEdge {
        public:
            FixedEdge(ParallelType paralle_type);

            virtual ~FixedEdge();
            // no use API in fixed edge.
            virtual ErrorCode SetQueueMaxSize(int queue_max_size) override;

            /*边的构建*/
            virtual ErrorCode Construct() override;

            virtual ErrorCode SetBuff(Buffer *buffer, bool is_external) override;
            virtual Buffer *CreateBuff() override;
            virtual bool NotifyWrite(Buffer *buffer) override;
            // virtual Buffer *GetBuff() override;
            virtual Buffer *GetBuff(const Node *node) override;

            virtual Buffer *GetGraphOutputBuffer() override; // 虚函数重载

            /**
             *   固定边设置数据，可以是cv::mat指针,buffer指针,或者其他数据指针.
             */

#ifdef ENABLE_3RD_OPENCV
            virtual ErrorCode SetMat(cv::Mat *mat, bool is_external) override;
            virtual cv::Mat *CreateMat(int rows, int cols, int type,
                                       const cv::Scalar &value) override;
            virtual bool NotifyWrite(cv::Mat *mat) override;
            virtual cv::Mat *GetMat(const Node *node) override;

            virtual cv::Mat *GetGraphOutputMat() override;
#endif

            virtual ErrorCode TakeDataPackage(DataPackage *data_pack) override;

            virtual EdgeUpdateFlag Update(const Node *node) override;

            virtual bool RequestTerminate() override;

            virtual int64_t GetIndex();

        private:
            // int index;
            DataPackage *data_packet_; // 数据包内可以是任何数据blob, cv::Mat, std::vector etc.
        };

    } // namespace dag
} // namespace rayshape

#endif
