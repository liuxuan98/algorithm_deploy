#ifndef _MODEL_SERIALIZE_H_
#define _MODEL_SERIALIZE_H_

#include "model/model.h"
#include "utils/codec/codec_include.h"

// Conditionally include and register model types based on compilation flags
#ifdef ENABLE_MNN_MODEL
#include "model/mnn/mnn_model.h"
CEREAL_REGISTER_TYPE(rayshape::MNNModel)
CEREAL_REGISTER_POLYMORPHIC_RELATION(rayshape::Model, rayshape::MNNModel)
#endif

#ifdef ENABLE_OPENVINO_MODEL
#include "model/openvino/openvino_model.h"
CEREAL_REGISTER_TYPE(rayshape::OpenVINOModel)
CEREAL_REGISTER_POLYMORPHIC_RELATION(rayshape::Model, rayshape::OpenVINOModel)
#endif

#ifdef ENABLE_ONNX_MODEL
#include "model/onnx/onnx_model.h"
CEREAL_REGISTER_TYPE(rayshape::ONNXModel)
CEREAL_REGISTER_POLYMORPHIC_RELATION(rayshape::Model, rayshape::ONNXModel)
#endif

#ifdef ENABLE_TENSORRT_MODEL
#include "model/tensorrt/tensorrt_model.h"
CEREAL_REGISTER_TYPE(rayshape::TensorRTModel)
CEREAL_REGISTER_POLYMORPHIC_RELATION(rayshape::Model, rayshape::TensorRTModel)
#endif

namespace rayshape
{
    template <class Archive> void serialize(Archive &archive, Model &model) {}

#ifdef ENABLE_MNN_MODEL
    template <typename Archive> void serialize(Archive &archive, MNNModel &model) {
        archive(model.cfg_str_, model.bin_buf_);
    }
#endif

#ifdef ENABLE_OPENVINO_MODEL
    template <typename Archive> void serialize(Archive &archive, OpenVINOModel &model) {
        archive(model.cfg_str_, model.xml_content_, model.bin_content_);
    }
#endif

#ifdef ENABLE_ONNX_MODEL
    template <typename Archive> void serialize(Archive &archive, ONNXModel &model) {
        archive(model.cfg_str_, model.bin_buf_);
    }
#endif

#ifdef ENABLE_TENSORRT_MODEL
    template <typename Archive> void serialize(Archive &archive, TensorRTModel &model) {
        archive(model.cfg_str_, model.engine_data_);
    }
#endif

} // namespace rayshape

#endif // _MODEL_SERIALIZE_H_
