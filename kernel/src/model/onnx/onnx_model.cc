#include "model/model_include.h"
#include "model/onnx/onnx_model.h"
#include "utils/codec/model_codec.h"
#include "base/logger.h"
#include <fstream>

namespace rayshape
{
    ONNXModel::ONNXModel() = default;
    ONNXModel::ONNXModel(const std::string &model_buf) : bin_buf_(model_buf) {}

    ModelType ONNXModel::GetModelType() const {
        return type_;
    }
} // namespace rayshape