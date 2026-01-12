#include "codec.h"

namespace rayshape
{
    // Decode::Decode() {}
    Decode::Decode(const std::string &name) : dag::Node(name) {}

    Decode::Decode(const std::string &name, std::vector<dag::Edge *> inputs,
                   std::vector<dag::Edge *> outputs) : dag::Node(name) {
        inputs_ = inputs;
        outputs_ = outputs;
    }

    Decode::~Decode() {}

} // namespace rayshape