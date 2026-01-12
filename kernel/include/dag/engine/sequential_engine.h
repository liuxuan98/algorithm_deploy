#ifndef DAG_SEQUENTIAL_ENGINE_H
#define DAG_SEQUENTIAL_ENGINE_H

#include "dag/engine.h"

/* 串行的DAG执行引擎
 */

namespace rayshape
{
    namespace dag
    {
        // 串行执行engine.
        class SequentialEngine: public ExecuteEngine {
        public:
            SequentialEngine();
            virtual ~SequentialEngine();

            virtual ErrorCode Init(std::vector<EdgeWrapper *> &edge_repository,
                                   std::vector<NodeWrapper *> &node_repository);

            virtual ErrorCode Setup();

            virtual ErrorCode DeInit();

            virtual ErrorCode Run();

        protected:
            std::vector<NodeWrapper *> topo_sort_node_;
            std::vector<EdgeWrapper *> edge_repository_;
        };

    } // namespace dag
} // namespace rayshape

#endif