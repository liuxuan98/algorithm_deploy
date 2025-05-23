// todo 直接在这个demo里面简单调用 ，不考虑在module里面在封装调用了

#include <iostream>
#include <memory>
#include <filesystem> //c++17
#include <algorithm>
#include <opencv2/opencv.hpp>
#include "inference/inference.h"

using namespace rayshape;

static ErrorCode MatToBlob(cv::Mat &src, Blob *dst)
{
    ErrorCode ret = RS_SUCCESS;

    int num_channel = src.channels();
    int rtype = CV_MAKETYPE(CV_32F, num_channel);
    src.convertTo(src, rtype);

    float *dst_ptr = (float *)dst->buffer->GetSrcData();
    if (dst_ptr == nullptr)
    {
        // log
        return RS_INVALID_PARAM;
    }
    switch (dst->data_format)
    {
    case DATA_FORMAT_NHWC:
    {
        memcpy(dst_ptr, src.data, src.total() * src.elemSize());
    }
    break;
    case DATA_FORMAT_NCHW:
    default:
    {
        int rows = src.rows;
        int cols = src.cols;
        std::vector<cv::Mat> channels;
        for (int c = 0; c < num_channel; c++)
        {
            cv::Mat tmp(rows, cols, CV_32FC1, (void *)dst_ptr);
            channels.emplace_back(tmp);
            dst_ptr += rows * cols;
        }
        cv::split(src, channels);
    }
    break;
    }

    return RS_SUCCESS;
}

static ErrorCode PreProcess(const cv::Mat &src, cv::Mat &dst)
{
    size_t input_h = 256;
    size_t input_w = 256;

    cv::Mat pr_img;
    cv::resize(src, pr_img, cv::Size(input_w, input_h));
    // resize_image(img, input_w, input_h);
    pr_img.convertTo(pr_img, CV_32FC3);
    pr_img = pr_img / 255.0;
    std::vector<cv::Mat> pr_img_channels;
    cv::split(pr_img, pr_img_channels);
    static constexpr double img_mean[3] = {0.185, 0.179, 0.174};
    static constexpr double img_std[3] = {0.179, 0.172, 0.171};
    for (int i = 0; i < 3; ++i)
    {
        pr_img_channels[i] = (pr_img_channels[i] - img_mean[i]) / img_std[i];
    }
    cv::merge(pr_img_channels, pr_img);

    dst = pr_img.clone();

    return RS_SUCCESS;
}

int main(int argc, char **argv)
{
    if (argc < 4)
    {
        std::cout << "Usage: " << argv[0] << " <img_path_dir> <ov_xml_path> <ov_bin_file>" << std::endl;

        return -1;
    }

    // 打印四个参数
    for (int i = 0; i < argc; ++i)
    {
        std::cout << "Argument " << i << ": " << argv[i] << std::endl;
    }

    std::string img_path_dir = argv[1];
    std::string ov_xml_path = argv[2];
    std::string ov_bin_file = argv[3];

    // 存放图像路径的列表
    std::vector<std::string> image_paths;

    // 遍历目录中的所有图片文件
    for (const auto &entry : std::filesystem::directory_iterator(img_path_dir))
    {
        if (entry.is_regular_file())
        {
            std::string ext = entry.path().extension().string();
            if (ext == ".jpg" || ext == ".jpeg" || ext == ".png" || ext == ".bmp")
            {
                image_paths.push_back(entry.path().string());
            }
        }
    }

    printf("this is a demo\n");
    ErrorCode ret = RS_SUCCESS;
    InferenceType inference_type = INFERENCE_TYPE_OPENVINO;
    std::shared_ptr<inference::Inference> inference = inference::CreateInference(inference_type);

    inference::Inference *inference_ptr = inference.get();
    ret = inference->Init(ov_xml_path, ov_bin_file);
    if (ret != RS_SUCCESS)
    {
        // log
        return -1;
    }
    Blob *input_blob = nullptr;
    std::string input_name = "inputs";
    ret = inference->InputBlobGet(input_name.c_str(), &input_blob);
    if (ret != RS_SUCCESS)
    {
        // log
        return -1;
    }
    const Blob *output_blob = nullptr;
    std::string output_name = "outputs";

    // inference::OpenVinoNetWork* = dynamic_cast 不能转化因为ov等网络是屏蔽了的 只有RS_PUBLIC的符号是导出的

    std::vector<std::string> label_str = {"thyroid", "breast", "others"};

    // 使用OpenCV读取和处理图像
    for (const auto &img_path : image_paths)
    {
        cv::Mat image = cv::imread(img_path, cv::IMREAD_COLOR);
        if (image.empty())
        {
            std::cerr << "Failed to load image: " << img_path << std::endl;
            continue;
        }
        cv::Mat input_mat;
        ret = PreProcess(image, input_mat);
        if (ret != RS_SUCCESS)
        {
            //
            return -1;
        }

        ret = MatToBlob(input_mat, input_blob);
        if (ret != RS_SUCCESS)
        {

            return -1;
        }

        // model infer
        ret = inference->Forward();
        if (ret != RS_SUCCESS)
        {
            printf("inference forward failed\n");
            return -1;
        }
        // post process
        ret = inference->OutputBlobGet(output_name.c_str(), &output_blob);

        int n = output_blob->dims.value[0];
        int c = output_blob->dims.value[1];

        float *output_ptr = (float *)output_blob->buffer->GetSrcData();

        std::vector<float> result = {0, 0, 0};

        memcpy(result.data(), output_ptr, n * c * sizeof(float));
        int max_index = std::distance(result.begin(), std::max_element(result.begin(), result.end()));
        // 获取标签
        std::string label = label_str[max_index];
        //  可以在这里添加更多处理逻辑
        cv::putText(image, label, cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 0), 2);

        // 显示图像
        cv::imshow("Result", image);
        cv::waitKey(0); // 每次按键显示下一张图
    }

    printf("demo end\n");
    return 0;
}
