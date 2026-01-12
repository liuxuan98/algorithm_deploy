#include "preprocess.h"




using namespace rayshape::dag;

namespace rayshape
{
    // 输入边包含数据为cv::Mat
    PreProcess::PreProcess(const std::string &name) : Node(name) {
        //自定义边
    
    }

    PreProcess::PreProcess(const std::string &name, std::vector<dag::Edge *> inputs,
                           std::vector<dag::Edge *> outputs) : Node(name, inputs, outputs) {}

    PreProcess::~PreProcess() {}

    ErrorCode PreProcess::Run() {
        ErrorCode ret = RS_SUCCESS;

        cv::Mat *src = (cv::Mat *)(inputs_[0]->GetMat());
        cv::Mat resize_img;
        cv::resize(*src, resize_img, cv::Size(img_h_, img_w_));
        // resize_image(img, input_w, input_h);
        resize_img.convertTo(resize_img, CV_32FC3, 1 / 255.0);

        std::vector<cv::Mat> resize_img_channels;
        cv::split(resize_img, resize_img_channels);
        for (int i = 0; i < 3; ++i) {
            resize_img_channels[i] = (resize_img_channels[i] - img_mean_[i]) / img_std_[i];
        }

        const cv::Scalar zero(0, 0, 0);
        cv::Mat *output_mat =
            outputs_[0]->CreateMat(resize_img.rows, resize_img.cols, resize_img.type(), zero);
        // 要支持内存复用
        // 自己边内部创建然后获取指针获得内容，这样边就具有管理能力
        cv::merge(resize_img_channels, *output_mat);

        outputs_[0]->NotifyWrite(output_mat);

        return ret;
    }

} // namespace rayshape