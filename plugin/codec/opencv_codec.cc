#include "opencv_codec.h"

namespace rayshape
{
    // OpenCvImageDecode::OpenCvImageDecode() {}
    OpenCvImageDecode::OpenCvImageDecode(const std::string &name) : Decode(name) {}

    OpenCvImageDecode::OpenCvImageDecode(const std::string &name, std::vector<dag::Edge *> inputs,
                                         std::vector<dag::Edge *> outputs) :
        Decode(name, inputs, outputs) {}

    OpenCvImageDecode::~OpenCvImageDecode() {}

    ErrorCode OpenCvImageDecode::Run() {
        return RS_SUCCESS;
    }

} // namespace rayshape