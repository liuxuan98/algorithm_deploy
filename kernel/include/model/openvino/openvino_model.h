#ifndef _OPENVINO_MODEL_H_
#define _OPENVINO_MODEL_H_

#include "model/model.h"
#include "base/common.h"
#include "base/macros.h"

namespace rayshape
{
    class RS_PUBLIC OpenVINOModel: public Model {
    public:
        static const ModelType type_;

        OpenVINOModel();
        explicit OpenVINOModel(const std::string &xml_buf, const std::string &bin_buf);
        ModelType GetModelType() const override;

    public:
        std::string cfg_str_;
        std::string xml_content_;
        std::string bin_content_;
    };
} // namespace rayshape

#endif // _OPENVINO_MODEL_H_