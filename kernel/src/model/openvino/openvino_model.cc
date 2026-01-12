#include "model/model_include.h"
#include "model/openvino/openvino_model.h"
#include "utils/codec/model_parse.h"

namespace rayshape
{
    const ModelType OpenVINOModel::type_ = ModelType::OPENVINO;

    OpenVINOModel::OpenVINOModel() = default;
    OpenVINOModel::OpenVINOModel(const std::string &xml_buf, const std::string &bin_buf) :
        xml_content_(xml_buf), bin_content_(bin_buf) {}

    ModelType OpenVINOModel::GetModelType() const {
        return type_;
    }
} // namespace rayshape
