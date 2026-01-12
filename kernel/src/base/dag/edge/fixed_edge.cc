#include "dag/edge/fixed_edge.h"

namespace rayshape
{
    namespace dag
    {
        TypeEdgeRegister<TypeEdgeCreator<FixedEdge>> g_fixed_edge_register(EDGE_TYPE_FIXED);

        FixedEdge::FixedEdge(ParallelType paralle_type) : AbstractEdge(paralle_type) {
            data_packet_ = new DataPackage();
        }

        FixedEdge::~FixedEdge() {
            delete data_packet_;
        }

        ErrorCode FixedEdge::SetQueueMaxSize(int queue_max_size) {
            RS_LOGI("fixed edge no need to set queue max size");
            return RS_NOT_IMPLEMENT;
        }

        ErrorCode FixedEdge::Construct() {
            return RS_SUCCESS;
        }

        ErrorCode FixedEdge::SetBuff(Buffer *buffer, bool is_external) {
            this->IncreaseIndex();          // 数据包索引加1
            data_packet_->SetIndex(index_); // 数据包设置最新索引
            return data_packet_->SetBuff(buffer, is_external);
        }
        // Not Finish.
        Buffer *FixedEdge::CreateBuff() {
            this->IncreaseIndex(); // 数据包索引加1
            data_packet_->SetIndex(index_);
            RSMemoryInfo tmp;
            return data_packet_->CreateBuff(nullptr, tmp);
        }

        bool FixedEdge::NotifyWrite(Buffer *buffer) {
            return data_packet_->NotifyWrite(buffer);
        }

        Buffer *FixedEdge::GetBuff(const Node *node) {
            return data_packet_->GetBuff();
        }

        Buffer *FixedEdge::GetGraphOutputBuffer() {
            return data_packet_->GetBuff();
        }

#ifdef ENABLE_3RD_OPENCV
        ErrorCode FixedEdge::SetMat(cv::Mat *mat, bool is_external) {
            this->IncreaseIndex(); // 数据包索引加1
            data_packet_->SetIndex(index_);
            return data_packet_->SetMat(mat, is_external);
        }

        cv::Mat *FixedEdge::CreateMat(int rows, int cols, int type, const cv::Scalar &value) {
            this->IncreaseIndex(); // 数据包索引加1
            data_packet_->SetIndex(index_);
            return data_packet_->CreateMat(rows, cols, type, value);
        }

        bool FixedEdge::NotifyWrite(cv::Mat *mat) {
            return data_packet_->NotifyWrite(mat);
        }

        cv::Mat *FixedEdge::GetMat(const Node *node) {
            return data_packet_->GetMat();
        }

        cv::Mat *FixedEdge::GetGraphOutputMat() {
            return data_packet_->GetMat();
        }

#endif

        ErrorCode FixedEdge::TakeDataPackage(DataPackage *data_pack) {
            if (data_packet_ != nullptr) {
                delete data_packet_;
            }
            data_packet_ = data_pack;
            return RS_SUCCESS;
        }

        EdgeUpdateFlag FixedEdge::Update(const Node *node = nullptr) {
            if (terminate_flag_) {
                return EdgeUpdateFlag::Terminate;
            } else {
                return EdgeUpdateFlag::Complete;
            }
        }

        bool FixedEdge::RequestTerminate() {
            terminate_flag_ = true;
            return true;
        }

        int64_t FixedEdge::GetIndex() {
            return data_pack_->GetIndex();
        }

    } // namespace dag
} // namespace rayshape