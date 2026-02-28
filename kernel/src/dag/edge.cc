#include "dag/edge.h"
#include "dag/util.h"

namespace rayshape
{
    namespace dag
    {
        Edge::Edge() {
            name_ = "edge_" + GetUniqueString();

            abstract_edge_ = CreateEdge(PARALLEL_TYPE_NONE);
            if (abstract_edge_ == nullptr) {
                RS_LOGE("out of memory!\n");
                return;
            }
        }

        Edge::Edge(const std::string &name) : name_(name) {
            if (name.empty()) {
                name_ = "edge_" + GetUniqueString();
            } else {
                name_ = name;
            }
            abstract_edge_ = CreateEdge(PARALLEL_TYPE_NONE);
            if (abstract_edge_ == nullptr) {
                RS_LOGE("out of memory!\n");
                return;
            }
        }

        Edge::~Edge() {
            if (abstract_edge_ != nullptr) {
                delete abstract_edge_;
                abstract_edge_ = nullptr;
            }
        }

        std::string Edge::GetName() const {
            return name_;
        }

        ErrorCode Edge::SetParallelType(const ParallelType &paralle_type) {
            ErrorCode ret = RS_SUCCESS;

            if (abstract_edge_ == nullptr) {
                abstract_edge_ = CreateEdge(paralle_type);
                if (abstract_edge_ == nullptr) {
                    RS_LOGE("out of memory!\n");
                    return RS_OUTOFMEMORY;
                }
            } else {
                abstract_edge_ = RecreateEdge(abstract_edge_, paralle_type);
                if (abstract_edge_ == nullptr) {
                    RS_LOGE("out of memory!\n");
                    return RS_OUTOFMEMORY;
                }
            }
            // 默认设置16 的缓冲区,对于流水线边来控制数据包缓冲区的最大容量.
            abstract_edge_->SetQueueMaxSize(16);

            return ret;
        }

        ParallelType Edge::GetParallelType() {
            return abstract_edge_->GetParallelType();
        }

        ErrorCode Edge::Construct() {
            return abstract_edge_->Construct();
        }

        ErrorCode Edge::SetBuff(Buffer *buffer, bool is_external) {
            return abstract_edge_->SetBuff(buffer, is_external); // todo
        }

        Buffer *Edge::CreateBuffer() {
            return abstract_edge_->CreateBuff(); // todo
        }

        bool Edge::NotifyWrite(Buffer *buffer) {
            return abstract_edge_->NotifyWrite(buffer); // todo
        }

        Buffer *Edge::GetBuff(const Node *node) {
            return abstract_edge_->GetBuff(node); // todo
        }

        Buffer *Edge::GetGraphOutputBuffer() {
            return abstract_edge_->GetGraphOutputBuffer();
        }

#ifdef ENABLE_3RD_OPENCV
        ErrorCode Edge::SetMat(cv::Mat *mat, bool is_external) {
            return abstract_edge_->SetMat(mat, is_external);
        }

        cv::Mat *Edge::CreateMat(int rows, int cols, int type, const cv::Scalar &value) {
            return abstract_edge_->CreateMat(rows, cols, type, value);
        }

        bool Edge::NotifyWrite(cv::Mat *mat) {
            return abstract_edge_->NotifyWrite(mat);
        }

        cv::Mat *Edge::GetMat(const Node *node) {
            return abstract_edge_->GetMat(node);
        }

        cv::Mat *Edge::GetGraphOutputMat() {
            return abstract_edge_->GetGraphOutputMat();
        }

#endif

        EdgeUpdateFlag Edge::Update(const Node *node) {
            return abstract_edge_->Update(node);
        }

        bool Edge::RequestTerminate() {
            return abstract_edge_->RequestTerminate();
        }

        ErrorCode Edge::IncreaseProducers(std::vector<Node *> &producers) {
            return abstract_edge_->IncreaseProducers(producers);
        }

        ErrorCode Edge::IncreaseConsumers(std::vector<Node *> &consumers) {
            return abstract_edge_->IncreaseConsumers(consumers);
        }

    } // namespace dag
} // namespace rayshape