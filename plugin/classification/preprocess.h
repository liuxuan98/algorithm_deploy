#ifndef PREPROCESS_H
#define PREPROCESS_H

#ifndef ENABLE_3RD_OPENCV
#define ENABLE_3RD_OPENCV

#endif // !ENABLE_3RD_OPENCV

#include "dag/node.h"

#include <opencv2/opencv.hpp>
// 2025.10.28 前处理接口基本上做完了
namespace rayshape
{
    class PreProcess: public dag::Node {
    public:
        PreProcess(const std::string &name);

        PreProcess(const std::string &name, std::vector<dag::Edge *> inputs,
                   std::vector<dag::Edge *> outputs);

        virtual ~PreProcess();

        virtual ErrorCode Run() override; //

    private:
        // rgv
        size_t img_h_ = 256;
        size_t img_w_ = 256;
        static constexpr double img_mean_[3] = {0.185, 0.179, 0.174};
        static constexpr double img_std_[3] = {0.179, 0.172, 0.171};
    };
} // namespace rayshape

#endif