#include "infer.h"
#include "model/model_manager.h"
#include "memory_manager/blob.h"
// include the opencv
#include <opencv2/opencv.hpp>
// infer 推理部分暂时都不支持动态尺寸推理
namespace rayshape  //rayshape
{
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

    ClassificationInfer::ClassificationInfer(const std::string &name) : dag::Node(name) {
        std::string desc_ =
            "input type cv::mat output type buffer,inference type is openvino cpu,static input";
    }

    ClassificationInfer::ClassificationInfer(const std::string &name,
                                             std::vector<dag::Edge *> inputs,
                                             std::vector<dag::Edge *> outputs) :
        dag::Node(name, inputs, outputs) {
        std::string desc_ =
            "input type cv::mat output type buffer,inference type is openvino cpu,static input";
    }

    ClassificationInfer::ClassificationInfer(const std::string &name, InferenceType type) :
        dag::Node(name) {
        std::string desc_ =
            "input type cv::mat output type buffer,inference type is openvino cpu,static input";
        type_ = type;

        inference_ = inference::CreateInference(type);
        if (inference_ == nullptr) {
            RS_LOGE("Failed to create inference.\n");
            // consr 构造完成的标志位.
        }
    }

    ClassificationInfer::ClassificationInfer(const std::string &name,
                                             std::vector<dag::Edge *> inputs,
                                             std::vector<dag::Edge *> outputs, InferenceType type) :
        dag::Node(name, inputs, outputs) {
        std::string desc_ =
            "input type cv::mat output type buffer,inference type is openvino cpu,static input";
        type_ = type;

        inference_ = inference::CreateInference(type);
        if (inference_ == nullptr) {
            RS_LOGE("Failed to create inference.\n");
            // consr 构造完成的标志位.
        }
    }

    ClassificationInfer::~ClassificationInfer() {}
    // 不允许使用继承成员
    ErrorCode ClassificationInfer::Init() {
        ErrorCode ret = RS_SUCCESS;
        std::string model_path = "D:/Program/rayshape_deploy/model/breast_thyroid/rsm/checkpoint-best-openvino.rsm";

        auto model = LoadModel(model_path);  //模型路径不正确

        CustomRuntime runtime;
        runtime.device_type_ = DeviceType::X86; // 使用CPU
        runtime.inference_type_ = type_;
        runtime.model_type_ = model->GetModelType(); // 使用模型的实际类型
        runtime.num_thread_ = 4;
        runtime.use_gpu_ = false;
        if (inference_ == nullptr) {
            inference_ = inference::CreateInference(type_);
            if (inference_ == nullptr) {
                RS_LOGE("Failed to create inference.\n");
                return RS_UNKNOWN;
            }
        }

        ret = inference_->Init(model.get(), &runtime);
        if (ret != RS_SUCCESS) {
            RS_LOGE("Failed to init inference.\n");
            return ret;
        }

        // 手动创建边？
        //  获取模型输入输出的描述
        //  inference_

        // 具备几个输入就创建几个输入边

        // for () {
        // }

        return RS_SUCCESS;
    }

    ErrorCode ClassificationInfer::DeInit() {
        ErrorCode ret = RS_SUCCESS;
        inference_->DeInit();

        return ret;
    }

    ErrorCode ClassificationInfer::SetInferenceType(InferenceType inference_type) {
        type_ = inference_type;

        return RS_SUCCESS;
    }

    ErrorCode ClassificationInfer::Run() {
        ErrorCode ret = RS_SUCCESS;
        std::vector<cv::Mat *> mats; // 只有一个输入,自己在node的run中确定
        for (auto input : inputs_) {
            cv::Mat *mat = input->GetMat(); // 节点的输入边获取数据
            mats.push_back(mat);
        }

        // 如果是动态尺寸
        // Blob *input_blob = nullptr;
        inference_input_names_ = {"inputs"}; //暂时先定死由自己定义.
        std::vector<Blob *> input_blobs(inference_input_names_.size(), nullptr);
        int i = 0;
        for (const std::string &str : inference_input_names_) {
            ret = inference_->InputBlobGet(str.c_str(), &input_blobs[i++]);
            if (ret != RS_SUCCESS) {
                RS_LOGE("Failed to get input blob.\n");
                return ret;
            }
        }

        for (int i = 0; i < mats.size(); i++) {
            cv::Mat *mat = mats[i];
            ret = MatToBlob(*mat, input_blobs[i]);
            if (ret != RS_SUCCESS) {
                RS_LOGE("Failed to convert mat to blob.\n");
                return ret;
            }
        }

        // model inference.
        ret = inference_->Forward(); // inference需要保留推理输入输出命名的接口和私有属性
        if (ret != RS_SUCCESS) {
            RS_LOGE("inference forward failed.\n");
            return ret;
        }
        // Get output /gain
        const Blob *output_blob = nullptr;
        std::string output_name = "outputs";
        ret = inference_->OutputBlobGet(output_name.c_str(), &output_blob);
        if (ret != RS_SUCCESS) {
            RS_LOGE("Failed to get output blob: %d\n", ret);
            return ret;
        }
        // keep do
        //  设置到输出边.blob中的buffer内存是由模型创建和管理的

        outputs_[0]->SetBuff(output_blob->buffer, true);

        return RS_SUCCESS;
    }
    // #TODO
    ClassificationPostProcess::ClassificationPostProcess(const std::string &name) :
        dag::Node(name) {
        std::string desc_ = "input edge packet buffer,output edge packet the result";
    }

    ClassificationPostProcess::ClassificationPostProcess(const std::string &name,
                                                         std::vector<dag::Edge *> inputs,
                                                         std::vector<dag::Edge *> outputs) :
        dag::Node(name, inputs, outputs) {
            
        std::string desc_ = "input edge packet buffer,output edge packet the result";
    
    }

    ClassificationPostProcess::~ClassificationPostProcess() {
        
    }

    ErrorCode ClassificationPostProcess::Run() {
        ErrorCode ret = RS_SUCCESS;
        std::vector<float> result = {0, 0, 0};
        int n = 1, c = 3;
        Buffer *buffer = inputs_[0]->GetBuff(); // todo

        float *output_ptr = (float *)RSBufferDataGet(buffer);
        // do softmax.
        if (buffer->GetMemoryType() == MemoryType::HOST) {
            memcpy(result.data(), output_ptr, n * c * sizeof(float));
        } else {
            RSMemoryInfo result_mem_info = buffer->GetMemoryInfo();
            result_mem_info.mem_type_ = MemoryType::HOST;
            Buffer result_buf(result.data(), result_mem_info);
            buffer->DeepCopy(result_buf);
        }
        RS_LOGI("result: %f, %f, %f", result[0], result[1], result[2]);
        int max_index =
            std::distance(result.begin(), std::max_element(result.begin(), result.end()));
        std::vector<std::string> label_str = {"thyroid", "breast", "others"};
        std::string label = label_str[max_index];
        RS_LOGI("label: %s", label.c_str());

        // ClassificationResult 数据结构 score label
        // ClassificationResult *result = new ClassificationResult(result[0], label);

        // 将数据放到输出边完成闭环
        return RS_SUCCESS;
    }

} // namespace rayshape