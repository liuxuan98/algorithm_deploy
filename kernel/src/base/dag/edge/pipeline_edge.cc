#include "dag/edge/pipeline_edge.h"

#include "dag/node.h"

namespace rayshape
{
    namespace dag
    {
        TypeEdgeRegister<TypeEdgeCreator<PipelineEdge>> g_fixed_edge_register(EDGE_TYPE_PIPELINE);

        PipelineEdge::PipelineEdge(ParallelType paralle_type) : AbstractEdge(paralle_type) {}

        PipelineEdge::~PipelineEdge() {
            consumers_size_ = 0;

            for (auto iter : data_packets_) {
                delete iter;
            }
            data_packets_.clear();

            consuming_dp_.clear();
            to_consume_index_.clear();
        }

        ErrorCode PipelineEdge::SetQueueMaxSize(int queue_max_size) {
            queue_max_size_ = queue_max_size;

            return RS_SUCCESS;
        }

        ErrorCode PipelineEdge::Construct() {
            // pipeline 边的构建,pipeline边也就一个生产者一个消费者
            consumers_size_ = static_cast<int>(consumers_.size());
            // 该边的消费者索引,消费者消费的最新索引,保持消费索引顺序
            for (auto iter : consumers_) {
                if (to_consume_index_.find(iter) == to_consume_index_.end()) {
                    to_consume_index_.insert({iter, 0});
                }
                // consuming_dp_ 记录该边消费者消费的当前数据包
                if (consuming_dp_.find(iter) == consuming_dp_.end()) {
                    consuming_dp_.insert({iter, nullptr});
                }
            }
            return RS_SUCCESS;
        }

        ErrorCode PipelineEdge::SetBuff(Buffer *buffer, bool is_external) {
            // 上锁
            std::unique_lock<std::mutex> lock(mutex_);
            if (std::find(consumers_.begin(), consumers_.end(), nullptr) == consumers_.end()) {
                queue_cv_.wait(lock, [this]() { return data_packets_.size() < queue_max_size_; });
            }

            PipelineDataPackage *data_pack = new PipelineDataPackage(consumers_size_);
            RS_CHECK_PARAM_NULL_RET_STATUS(data_pack, "PipelineDataPackage is null.\n");
            this->IncreaseIndex();
            data_pack->SetIndex(index_);

            data_packets_.push_back(data_pack);
            cv_.notify_all();

            ErrorCode ret = data_pack->SetBuff(buffer, is_external);
            RS_RETURN_ON_NEQ(ret, RS_SUCCESS, "data_pack SetBuff error.\n");

            return ret;
        }

        Buffer *PipelineEdge::CreateBuff() {
            // 支持各类设备的一个创建buffer #TODO：暂时用空接口

            return nullptr;
        }

        bool PipelineEdge::NotifyWrite(Buffer *buffer) {
            std::lock_guard<std::mutex> lock(mutex_);
            bool is_notify = false;
            for (auto iter = data_packets_.rbegin(); iter != data_packets_.rend(); ++iter) {
                if ((*iter)->NotifyWrite(buffer)) {
                    is_notify = true;
                    break;
                }
            }
            if (!is_notify) {
                RS_LOGE("This buffer[%p] is error.\n", buffer);
            }
            return is_notify;
        }
        // 重够getbuffer接口,从节点中获取pipeline数据包
        Buffer *PipelineEdge::GetBuff(const Node *node) {
            PipelineDataPackage *data_pack = GetPipelineDataPacket(node);
            if (data_pack == nullptr) {
                RS_LOGE("GetPipelineDataPacket error.\n");
                return nullptr;
            }

            Node *tmp_node = const_cast<Node *>(node);
            if (consuming_dp_.find(tmp_node) != consuming_dp_.end()) {
                return data_pack->GetBuff(); // 当前消费节点获取数据包
            } else {
                return data_pack->GetBufferDirect();
            }
        }

        Buffer *PipelineEdge::GetGraphOutputBuffer() {
            PipelineDataPackage *dp = nullptr;
            EdgeUpdateFlag update_flag = Update(nullptr);
            if (update_flag == EdgeUpdateFlag::Terminate) {
                RS_LOGI("User voluntarily terminates.\n");
            } else if (update_flag == EdgeUpdateFlag::Error) {
                RS_LOGI("getGraphOutput update error.\n");
            } else {
                dp = GetPipelineDataPacket(nullptr);
            }
            if (dp == nullptr) {
                RS_LOGE("PipelineDataPacket is null, this edge is not output edge.\n");
                return nullptr;
            }

            return dp->GetBuff();
        }

#ifdef ENABLE_3RD_OPENCV
        ErrorCode PipelineEdge::SetMat(cv::Mat *mat, bool is_external) {
            std::unique_lock<std::mutex> lock(mutex_);
            // 无消费者则等待
            if (std::find(consumers_.begin(), consumers_.end(), nullptr) == consumers_.end()) {
                queue_cv_.wait(lock, [this]() { return data_packets_.size() < queue_max_size_; });
            }

            PipelineDataPackage *data_pack = new PipelineDataPackage(consumers_size_);
            RS_CHECK_PARAM_NULL_RET_STATUS(data_pack, "PipelineDataPackage is null.\n");
            this->IncreaseIndex();
            data_pack->SetIndex(index_);

            data_packets_.push_back(data_pack); // 消费包进队
            cv_.notify_all();                   //

            ErrorCode ret = data_pack->SetMat(mat, is_external);
            RS_RETURN_ON_NEQ(ret, RS_SUCCESS, "PipelineDataPackage set error.\n");

            return ret;
        }

        cv::Mat *PipelineEdge::CreateMat(int rows, int cols, int type, const cv::Scalar &value) {
            std::unique_lock<std::mutex> lock(mutex_);
            if (std::find(consumers_.begin(), consumers_.end(), nullptr) == consumers_.end()) {
                queue_cv_.wait(lock, [this]() { return data_packets_.size() < queue_max_size_; });
            }

            PipelineDataPackage *data_pack = new PipelineDataPackage(consumers_size_);
            if (data_pack == nullptr) {
                RS_LOGE("PipelineDataPackage is null.\n");
                return nullptr;
            }
            // RS_CHECK_PARAM_NULL_RET_STATUS(data_pack, "PipelineDataPackage is null.\n");

            this->IncreaseIndex();
            data_pack->SetIndex(index_);
            data_packets_.push_back(data_pack);
            cv_.notify_all();

            cv::Mat *ret_mat = data_pack->CreateMat(rows, cols, type, value);
            if (ret_mat == nullptr) {
                RS_LOGE("data_pack CreateMat is null.\n");
                return nullptr;
            }
            // RS_CHECK_PARAM_NULL_RET_STATUS(ret_mat, "data_pack CreateMat is null.\n");

            return ret_mat;
        }

        bool PipelineEdge::NotifyWrite(cv::Mat *mat) {
            std::lock_guard<std::mutex> lock(mutex_);
            bool is_notify = false;
            for (auto iter = data_packets_.rbegin(); iter != data_packets_.rend(); ++iter) {
                if ((*iter)->NotifyWrite(mat)) {
                    is_notify = true;
                    break;
                }
            }
            if (!is_notify) {
                RS_LOGE("This mat[%p] is error.\n", mat);
            }
            return is_notify;
        }

        cv::Mat *PipelineEdge::GetMat(const Node *node) {
            PipelineDataPackage *data_pack = GetPipelineDataPacket(node);
            if (data_pack == nullptr) {
                RS_LOGE("GetPipelineDataPacket error.\n");
                return nullptr;
            }
            Node *tmp_node = const_cast<Node *>(node);
            if (consuming_dp_.find(tmp_node) != consuming_dp_.end()) {
                return data_pack->GetMat();
            } else {
                return data_pack->GetMatDirect();
            }
        }

        cv::Mat *PipelineEdge::GetGraphOutputMat() {
            PipelineDataPackage *data_pack = nullptr;
            EdgeUpdateFlag update_flag = Update(nullptr);
            if (update_flag == EdgeUpdateFlag::Terminate) {
                RS_LOGI("User voluntarily terminates.\n");
            } else if (update_flag == EdgeUpdateFlag::Error) {
                RS_LOGI("getGraphOutput update error.\n");
            } else {
                data_pack = GetPipelineDataPacket(nullptr);
            }
            if (data_pack == nullptr) {
                RS_LOGE("PipelineDataPacket is null, this edge is not output edge.\n");
                return nullptr;
            }

            return data_pack->GetMat();
        }

#endif

        ErrorCode PipelineEdge::TakeDataPackage(DataPackage *data_pack) {
            // 上锁
            std::unique_lock<std::mutex> lock(mutex_);
            if (std::find(consumers_.begin(), consumers_.end(), nullptr) == consumers_.end()) {
                queue_cv_.wait(lock, [this]() { return data_packets_.size() < queue_max_size_; });
            }
            PipelineDataPackage *dp = new PipelineDataPackage(consumers_size_);
            RS_CHECK_PARAM_NULL_RET_STATUS(dp, "PipelineDataPackage is null.\n");

            data_packets_.push_back(dp);
            cv_.notify_all();

            ErrorCode ret = dp->TakeDataPackage(data_pack);
            RS_RETURN_ON_NEQ(ret, RS_SUCCESS, "PipelineDataPackage take error.\n");

            return ret;
        }

        // 多消费者场景下数据包的安全消费和回收,pipeline场景下异步线程调度每个节点导致多个消费者存在竞争，需要保持数据包安全
        EdgeUpdateFlag PipelineEdge::Update(const Node *node) {
            Node *tmp_node = const_cast<Node *>(node);
            if (!CheckNode(tmp_node)) {
                RS_LOGE("This node is error.\n");
                return EdgeUpdateFlag::Error;
            }

            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait(lock, [this, tmp_node] {
                return to_consume_index_[tmp_node] < data_packets_.size() || terminate_flag_;
            });
            // 1.加锁并等待,当前节点有消费包(已存在)可消费，否则等待
            // 2.中止标志, 确定中止flag为true,否则等待
            if (terminate_flag_) {
                return EdgeUpdateFlag::Terminate;
            }
            // 找消费包
            PipelineDataPackage *dp = nullptr;
            int index = to_consume_index_[tmp_node]; // 找到消费节点的消费包索引
            int count = 0;                           // 消费者节点计数,一般只有一个node
            auto iter = data_packets_.begin();       // 消费包列表
                                                     // 遍历list找消费包
            for (int i = 0; i < index; i++) {
                if ((*iter)->GetConsumersCount() == consumers_size_) {
                    count++; // 计算需要删除的数据包
                }
                iter++;
            }

            dp = (*iter);
            dp->IncreaseConsumersCount(); // 增加数据包的消费者计数
            consuming_dp_[tmp_node] = dp; // 记录节点当前消费的数据包

            // update
            int real_count = 0;
            iter = data_packets_.begin();
            for (int i = 0; i < count; i++) {
                bool delete_flag = true;
                // 消费者操作是并发的，可能有一个消费者正在使用此数据包
                for (auto consuming_dp : consuming_dp_) {
                    if (consuming_dp.second == *iter) {
                        delete_flag = false; // 迭代需要删掉的数据包中是否还有消费者正在使用的
                        break;
                    }
                }
                if (delete_flag) {
                    delete (*iter); // 删掉数据包obj
                    iter++;         // 迭代下一个数据包
                    real_count++;   // 标记为可删除pipe包都是顺序的
                } else {
                    break;
                }
            }

            // 销毁不会被使用到的数据
            if (real_count > 0) {
                data_packets_.erase(data_packets_.begin(), iter);
                for (auto &iter : to_consume_index_) {
                    iter.second -= real_count;
                    // 删除数据包后，更新所有消费节点的消费索引,减去删除的数量
                }
                // 生产队列
                if (data_packets_.size() + real_count >= queue_max_size_
                    && data_packets_.size() < queue_max_size_) {
                    queue_cv_.notify_all(); // 通知生产者线程生产？
                }
            }

            // 让下一次数据没有的时候线程一直在等待
            // 增加当前节点的消费索引,下次该节点就会消费下一个数据包
            to_consume_index_[tmp_node]++;
            return EdgeUpdateFlag::Complete;
        }

        bool PipelineEdge::RequestTerminate() {
            std::unique_lock<std::mutex> lock(mutex_);
            terminate_flag_ = true;
            cv_.notify_all();
            return true;
        }

        PipelineDataPackage *PipelineEdge::GetPipelineDataPacket(const Node *node) {
            Node *tmp_node = const_cast<Node *>(node);
            if (consuming_dp_.find(tmp_node) != consuming_dp_.end()) {
                auto iter = consuming_dp_.find(tmp_node);
                if (iter->second == nullptr) {
                    RS_LOGE("node[%s] is error!\n", tmp_node->GetName().c_str());
                    return nullptr;
                }
                return iter->second;
            } else if (std::find(producers_.begin(), producers_.end(), tmp_node)
                       != producers_.end()) {
                return data_packets_.back();
            } else {
                RS_LOGE("node[%s] is error!\n", tmp_node->GetName().c_str());
                return nullptr;
            }
        }

    } // namespace dag
} // namespace rayshape