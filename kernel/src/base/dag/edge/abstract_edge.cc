#include "dag/edge/abstract_edge.h"
#include "dag/util.h"

namespace rayshape
{
    namespace dag
    {
        AbstractEdge::AbstractEdge(ParallelType paralle_type) : parallel_type_(paralle_type) {}

        AbstractEdge::~AbstractEdge() {
            producers_.clear();
            consumers_.clear();
        }

        ParallelType AbstractEdge::GetParallelType() {
            return parallel_type_;
        }

        std::vector<Node *> AbstractEdge::GetProducers() {
            return producers_;
        }

        ErrorCode AbstractEdge::IncreaseProducers(std::vector<Node *> &producers) {
            for (auto iter : producers) {
                InsertUnique(producers_, iter);
            }
            return RS_SUCCESS;
        }

        std::vector<Node *> AbstractEdge::GetConsumers() {
            return consumers_;
        }

        ErrorCode AbstractEdge::IncreaseConsumers(std::vector<Node *> &consumers) {
            for (auto iter : consumers) {
                InsertUnique(consumers_, iter);
            }
            return RS_SUCCESS;
        }

        void AbstractEdge::IncreaseIndex() {
            if (index_ != INT64_MAX) {
                index_++;
            } else {
                index_ = 0;
            }
        }

        bool AbstractEdge::CheckNode(const Node *node) {
            if (std::find(consumers_.begin(), consumers_.end(), node) != consumers_.end()) {
                return true;
            } else {
                if (node != nullptr) {
                    Node *tmp_node = const_cast<Node *>(node);
                    RS_LOGE("This node[%s] is error.\n", tmp_node->GetName().c_str());
                } else {
                    RS_LOGE("This node is error.\n");
                }
                return false;
            }
        }

        template <typename T>
        ErrorCode AbstractEdge::SetAny(T *t, bool is_external = true) {
            DataPackage *data_packet = new DataPackage();
            if (data_packet == nullptr) {
                RS_LOGE("failed to create data packet");
                return RS_OUTOFMEMORY;
            }
            /**
             * 从外部设置数据进入边内
             */
            ErrorCode ret = data_packet->SetAny<T>(t, is_external);
            if (ret != RS_SUCCESS) {
                RS_LOGE("failed to set any\n");
                delete data_packet;
                return ret;
            }
            ret = this->TakeDataPackage(data_packet);
            if (ret != RS_SUCCESS) {
                RS_LOGE("failed to take data package\n");
                delete data_packet;
                return ret;
            }

            return RS_SUCCESS;
        }

        template <typename T>
        T *AbstractEdge::GetAny(const Node *node) {
            DataPackage *data_packet = this->getDataPackage(node);
            if (data_packet == nullptr) {
                RS_LOGE("Failed to get data packet.\n");
                return nullptr;
            }

            if (parallel_type_ == PARALLEL_TYPE_PIPELINE) {
                // todo

            } else {
                T *t = data_packet->get<T>();
                if (t == nullptr) {
                    RS_LOGE("Failed to get any.\n");
                    return nullptr;
                }
                return t;
            }
        }

        std::map<EdgeType, std::shared_ptr<EdgeCreator>> &GetGlobalEdgeCreatorMap() {
            static std::once_flag once;
            static std::shared_ptr<std::map<EdgeType, std::shared_ptr<EdgeCreator>>> creators;
            std::call_once(once, []() {
                creators.reset(new std::map<EdgeType, std::shared_ptr<EdgeCreator>>);
            });
            return *creators;
        }
        /**
         * 根据graph中并行类型创建对应的edge,边类型只有2种普通边和流水线边,流水线边有序控制数据流动
         */
        EdgeType GetEdgeType(ParallelType type) {
            switch (type) {
            case PARALLEL_TYPE_NONE:
                return EDGE_TYPE_FIXED;
            case PARALLEL_TYPE_SEQUENTIAL:
                return EDGE_TYPE_FIXED;
            case PARALLEL_TYPE_TASK:
                return EDGE_TYPE_FIXED;
            case PARALLEL_TYPE_PIPELINE:
                return EDGE_TYPE_PIPELINE;
            default:
                return EDGE_TYPE_FIXED;
            }
        }

        AbstractEdge *CreateEdge(ParallelType type) {
            AbstractEdge *temp = nullptr;
            auto &creater_map = GetGlobalEdgeCreatorMap();
            EdgeType edge_type = GetEdgeType(type);
            if (creater_map.count(edge_type) > 0) {
                temp = creater_map[edge_type]->CreateEdge(type);
            }
            return temp;
        }

        AbstractEdge *RecreateEdge(AbstractEdge *abstract_edge, const ParallelType &paralle_type) {
            // todo
            ParallelType cur_paralle_type = abstract_edge->GetParallelType();
            AbstractEdge *new_abstract_edge = nullptr;
            if ((int)paralle_type < (int)PARALLEL_TYPE_PIPELINE
                && (int)cur_paralle_type < (int)PARALLEL_TYPE_PIPELINE) {
                new_abstract_edge = abstract_edge;
                // 重创建的并行类型不等于当前的并行类型
            } else if ((int)paralle_type > (int)cur_paralle_type) {
                new_abstract_edge = CreateEdge(paralle_type);
                if (new_abstract_edge == nullptr) {
                    RS_LOGE("out of memory!\n");
                    return nullptr;
                }
                std::vector<Node *> producers = abstract_edge->GetProducers();
                new_abstract_edge->IncreaseProducers(producers);
                std::vector<Node *> consumers = abstract_edge->GetConsumers();
                new_abstract_edge->IncreaseConsumers(consumers);
                delete abstract_edge;
            } else {
                new_abstract_edge = abstract_edge;
            }
            return new_abstract_edge;
        }

    } // namespace dag
} // namespace rayshape