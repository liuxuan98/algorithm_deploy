#ifndef _MODEL_CODEC_H_
#define _MODEL_CODEC_H_

#include "model/model.h"
#include "utils/codec/codec_include.h"

namespace rayshape
{

    class ModelCodec {
    public:
        ModelCodec() = default;
        ModelCodec(std::unique_ptr<Model> model) : model_(std::move(model)) {}

    protected:
        friend class cereal::access;
        template <typename Archive> void serialize(Archive &ar) {
            ar(model_);
        }

    public:
        std::unique_ptr<Model> model_ = nullptr;
    };

} // namespace rayshape

#endif // _MODEL_CODEC_H_