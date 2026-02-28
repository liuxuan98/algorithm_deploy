#include "dag/engine/sequential_engine.h"
#include "dag/util.h"

namespace rayshape
{
    namespace dag
    {
        SequentialEngine::SequentialEngine() : ExecuteEngine() {}

        SequentialEngine::~SequentialEngine() {}

        ErrorCode SequentialEngine::Init(std::vector<EdgeWrapper *> &edge_repository,
                                         std::vector<NodeWrapper *> &node_repository) {
            return RS_SUCCESS;
        }

        ErrorCode SequentialEngine::DeInit() {
            return RS_SUCCESS;
        }

        ErrorCode SequentialEngine::Setup() {
            return RS_SUCCESS;
        }

        ErrorCode SequentialEngine::Run() {
            ErrorCode ret = RS_SUCCESS;

            return ret;
        }

    } // namespace dag
} // namespace rayshape