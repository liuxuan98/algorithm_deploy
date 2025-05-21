#include "inference/inference.h"

namespace rayshape
{
    namespace inference
    {
        Inference::Inference(InferenceType type)
        {
            type_ = type;
        }

        Inference::~Inference() {}

        InferenceType Inference::GetInferenceType()
        {
            return type_;
        }

        std::map<InferenceType, std::shared_ptr<InferenceCreator>> &
        GetGlobalInferenceCreatorMap()
        {
            static std::once_flag once;
            static std::shared_ptr<
                std::map<InferenceType, std::shared_ptr<InferenceCreator>>>
                creators;
            std::call_once(once, []()
                           { creators.reset(
                                 new std::map<InferenceType, std::shared_ptr<InferenceCreator>>); });
            return *creators;
        }

        std::shared_ptr<Inference> CreateInference(InferenceType type)
        {
            std::shared_ptr<Inference> temp = nullptr;
            auto &creater_map = GetGlobalInferenceCreatorMap();
            if (creater_map.count(type) > 0)
            {
                temp = creater_map[type]->CreateInference(type);
            }
            return temp;
        }

    }
}