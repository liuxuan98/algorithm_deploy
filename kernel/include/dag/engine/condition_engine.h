// #ifndef _DAG_CONDITION_ENGINE_H_
// #define _DAG_CONDITION_ENGINE_H_

// #include "dag/engine.h"

// namespace rayshape
// {
//     namespace dag
//     {
//         class ConditionEngine: public ExecuteEngine {
//         public:
//             ConditionEngine(const std::string &name, const std::string &description);

//             virtual ErrorCode Init(std::vector<EdgeWrapper *> &edge_repository,
//                                    std::vector<NodeWrapper *> &node_repository) override;

//             virtual ErrorCode Setup() override;

//             virtual ErrorCode DeInit() override;

//             virtual ErrorCode Run() override;

//         protected:
//             int index_ = -1;
//             Node *condition_ = nullptr;
//             std::vector<NodeWrapper *> node_repository_;
//             std::vector<EdgeWrapper *> edge_repository_;

//             // private:
//             //     std::string m_condition_name;
//         };

//     } // namespace dag
// } // namespace rayshape

// #endif