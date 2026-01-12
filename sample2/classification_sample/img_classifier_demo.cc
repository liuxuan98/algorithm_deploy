// todo 直接在这个demo里面简单调用 ，不考虑在module里面在封装调用了

#include <iostream>
#include <memory>
#include <vector>
// #include <filesystem> //c++17
#include <algorithm>
#include <opencv2/opencv.hpp>
#include "inference/inference.h"
#include "base/logger.h"
#include "sample_file_utils.h"
#include "model/model_manager.h"

using namespace rayshape;

static ErrorCode MatToBlob(cv::Mat &src, Blob *dst) {
    ErrorCode ret = RS_SUCCESS;

    int num_channel = src.channels();
    int rtype = CV_MAKETYPE(CV_32F, num_channel);
    src.convertTo(src, rtype);

    float *dst_ptr = (float *)RSBufferDataGet(dst->buffer);
    //(float *)dst->buffer->GetDataPtr();
    if (dst_ptr == nullptr) {
        // log
        return RS_INVALID_PARAM;
    }
    switch (dst->data_format) {
    case DataFormat::NHWC: {
        if (dst->buffer->GetMemoryType() == MemoryType::HOST) {
            memcpy(dst_ptr, src.data, src.total() * src.elemSize());
        } else {
            RSMemoryInfo src_mem_info = dst->buffer->GetMemoryInfo();
            src_mem_info.mem_type_ = MemoryType::HOST;
            Buffer src_buf(src.data, src_mem_info);
            src_buf.DeepCopy(*dst->buffer);
        }
    } break;
    case DataFormat::NCHW:
    default: {
        if (dst->buffer->GetMemoryType() == MemoryType::HOST) {
            int rows = src.rows;
            int cols = src.cols;
            std::vector<cv::Mat> channels;
            for (int c = 0; c < num_channel; c++) {
                cv::Mat tmp(rows, cols, CV_32FC1, (void *)dst_ptr);
                channels.emplace_back(tmp);
                dst_ptr += rows * cols;
            }
            cv::split(src, channels);
        } else {
            RSMemoryInfo src_mem_info = dst->buffer->GetMemoryInfo();
            src_mem_info.mem_type_ = MemoryType::HOST;
            Buffer src_buf(src_mem_info);
            float *src_ptr = (float *)RSBufferDataGet(&src_buf);

            int rows = src.rows;
            int cols = src.cols;
            std::vector<cv::Mat> channels;
            for (int c = 0; c < num_channel; c++) {
                cv::Mat tmp(rows, cols, CV_32FC1, (void *)src_ptr);
                channels.emplace_back(tmp);
                src_ptr += rows * cols;
            }
            cv::split(src, channels);

            src_buf.DeepCopy(*dst->buffer);
        }
    } break;
    }

    return RS_SUCCESS;
}

static ErrorCode PreProcess(const cv::Mat &src, cv::Mat &dst) {
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
    for (int i = 0; i < 3; ++i) {
        pr_img_channels[i] = (pr_img_channels[i] - img_mean[i]) / img_std[i];
    }
    cv::merge(pr_img_channels, pr_img);

    dst = pr_img.clone();

    return RS_SUCCESS;
}

int main(int argc, char **argv) {
    if (argc < 4) {
        std::cout << "Usage: " << argv[0]
                  << " <img_path_dir> <serialized_model_path> <inference_type>" << std::endl;
        std::cout << "Note: model_path should be a serialized model file" << std::endl;
        return -1;
    }

    auto rs_ret = 0;
    InitRSLogSystem();
    LogFileSink::Instance().Init();
    RS_LOGD("main begin :%d\n", rs_ret);

    // 打印三个参数
    for (int i = 0; i < argc; ++i) {
        std::cout << "Argument " << i << ": " << argv[i] << std::endl;
    }

    std::string img_path_dir = argv[1];
    std::string model_path = argv[2];
    std::string inference_type_str = argv[3];
    static const std::unordered_map<std::string, InferenceType> INFERENCE_TYPE_MAP = {
        {"mnn", InferenceType::MNN},
        {"openvino", InferenceType::OPENVINO},
        {"onnxruntime", InferenceType::ONNXRUNTIME},
        {"tensorrt", InferenceType::TENSORRT}};
    auto it = INFERENCE_TYPE_MAP.find(inference_type_str);
    if (it == INFERENCE_TYPE_MAP.end()) {
        std::cerr << "Unsupported inference type: " << inference_type_str << std::endl;
        return -1;
    }

    // 存放图像路径的列表
    std::vector<std::string> image_names = getFilesInDirectory(img_path_dir);

    printf("this is a demo\n");
    ErrorCode ret = RS_SUCCESS;
    InferenceType inference_type = it->second;
    std::shared_ptr<inference::Inference> inference = inference::CreateInference(inference_type);

    if (inference == nullptr) {
        RS_LOGE("Failed to create inference instance\n");
        return -1;
    }

    // 使用ModelManager加载序列化模型
    auto model = LoadModel(model_path);
    if (!model) {
        RS_LOGE("Failed to load model from: %s\n", model_path.c_str());
        return -1;
    }

    // 创建运行时配置
    CustomRuntime runtime;
    runtime.device_type_ = DeviceType::X86; // 使用CPU
    runtime.inference_type_ = inference_type;
    runtime.model_type_ = model->GetModelType(); // 使用模型的实际类型
    runtime.num_thread_ = 4;
    runtime.use_gpu_ = false;

    // 初始化推理引擎
    ret = inference->Init(model.get(), &runtime);
    if (ret != RS_SUCCESS) {
        RS_LOGE("Failed to initialize inference: %d\n", ret);
        return -1;
    }

    Blob *input_blob = nullptr;
    std::string input_name = "inputs";
    ret = inference->InputBlobGet(input_name.c_str(), &input_blob);
    if (ret != RS_SUCCESS) {
        RS_LOGE("Failed to get input blob: %d\n", ret);
        return -1;
    }

    const Blob *output_blob = nullptr;
    std::string output_name = "outputs";

    std::vector<std::string> label_str = {"thyroid", "breast", "others"};

    // 使用OpenCV读取和处理图像
    for (const auto &img_name : image_names) {
        std::string full_path = img_path_dir + "/" + img_name;
        cv::Mat image = cv::imread(full_path, cv::IMREAD_COLOR);
        if (image.empty()) {
            std::cerr << "Failed to load image: " << full_path << std::endl;
            continue;
        }

        cv::Mat input_mat;
        ret = PreProcess(image, input_mat);
        if (ret != RS_SUCCESS) {
            RS_LOGE("Failed to preprocess image: %d\n", ret);
            return -1;
        }

        ret = MatToBlob(input_mat, input_blob);
        if (ret != RS_SUCCESS) {
            RS_LOGE("Failed to convert Mat to Blob: %d\n", ret);
            return -1;
        }

        // model infer
        ret = inference->Forward();
        if (ret != RS_SUCCESS) {
            RS_LOGE("inference forward failed: %d\n", ret);
            return -1;
        }

        // post process
        ret = inference->OutputBlobGet(output_name.c_str(), &output_blob);
        if (ret != RS_SUCCESS) {
            RS_LOGE("Failed to get output blob: %d\n", ret);
            return -1;
        }

        int n = output_blob->dims.value[0];
        int c = output_blob->dims.value[1];

        float *output_ptr = (float *)RSBufferDataGet(output_blob->buffer);
        if (output_ptr == nullptr) {
            RS_LOGE("Failed to get output buffer data\n");
            return -1;
        }

        std::vector<float> result = {0, 0, 0};
        if (output_blob->buffer->GetMemoryType() == MemoryType::HOST) {
            memcpy(result.data(), output_ptr, n * c * sizeof(float));
        } else {
            RSMemoryInfo result_mem_info = output_blob->buffer->GetMemoryInfo();
            result_mem_info.mem_type_ = MemoryType::HOST;
            Buffer result_buf(result.data(), result_mem_info);
            output_blob->buffer->DeepCopy(result_buf);
        }
        RS_LOGI("result: %f, %f, %f", result[0], result[1], result[2]);

        int max_index =
            std::distance(result.begin(), std::max_element(result.begin(), result.end()));
        // 获取标签
        std::string label = label_str[max_index];

        // 可以在这里添加更多处理逻辑
        cv::putText(image, label, cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1.0,
                    cv::Scalar(0, 255, 0), 2);

        // 显示图像
        // cv::imshow("Result", image);
        // cv::waitKey(0); // 每次按键显示下一张图
        cv::imwrite("output/" + inference_type_str + "/result_" + img_name, image);
    }

    printf("demo end\n");
    return 0;
}
