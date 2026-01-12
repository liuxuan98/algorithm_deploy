#ifndef _MNN_MODEL_H_
#define _MNN_MODEL_H_

#include "model/model.h"
#include "base/common.h"
#include "base/macros.h"

namespace rayshape
{
    class RS_PUBLIC MNNModel: public Model {
    public:
        static const ModelType type_ = ModelType::MNN;

        MNNModel();
        explicit MNNModel(const std::string &model_buf);
        ModelType GetModelType() const override;

    public:
        std::string cfg_str_;
        std::string bin_buf_;
    };
} // namespace rayshape

#endif // _MNN_MODEL_H_
