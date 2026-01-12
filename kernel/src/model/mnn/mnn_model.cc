#include "model/model_include.h"
#include "model/mnn/mnn_model.h"
#include "utils/codec/model_codec.h"
#include "base/logger.h"
#include <fstream>

namespace rayshape
{
    MNNModel::MNNModel() = default;
    MNNModel::MNNModel(const std::string &model_buf) : bin_buf_(model_buf) {}

    ModelType MNNModel::GetModelType() const {
        return type_;
    }
} // namespace rayshape