// 调用插件的部分

#include <iostream>
#include "classification/classification.h" //引入plugin的源代码文件
#include "dag/edge.h"
// #include <memory>

using namespace rayshape;

int demo1(std::string img_path, std::string model_path, std::string inference_type);
// pipeline scheduling demo.
int PipelineScheduling(std::string img_path, std::string model_path, std::string inference_type);

int main1(int argc, char **argv) {
    if (argc < 4) {
        std::cout << "Usage: " << argv[0]
                  << " <img_path/video_path> <serialized_model_path> <inference_type>" << std::endl;
        std::cout << "Note: model_path should be a serialized model file" << std::endl;
        return -1;
    }

    std::string img_path = argv[1];
    std::string model_path = argv[2];
    std::string inference_type = argv[3];

    std::cout << "begin build classificaiton graph!" << std::endl;
    std::string graph_name = "Thyroid_Breast_classification";

    rayshape::InferenceType infer_type = InferenceType::OPENVINO;

    ParallelType parall_type = PARALLEL_TYPE_TASK;

    // 已默认构建方式进行创建
    // demo1(img_path, model_path, inference_type);

    // 创建 unique_ptr 管理的 Edge 对象
    std::unique_ptr<dag::Edge> input_edge = std::make_unique<dag::Edge>("input_edge"); // 图的输入变
    std::unique_ptr<dag::Edge> output_edge = std::make_unique<dag::Edge>("output_edge");

    rayshape::ClassificationGraph graph_demo(
        "classification_demo", {input_edge.get()},
        {output_edge.get()}); // 将构造中的节点构造放到make graph中

    graph_demo.SetParallelType(parall_type);

    ErrorCode status = graph_demo.MakeGraph(infer_type); // 构建图 #基本完成简单图的构建
    if (status != RS_SUCCESS) {
        RS_LOGE("graph MakeGraph failed\n");
        return -1;
    }

    status = graph_demo.Init();
    if (status != RS_SUCCESS) {
        RS_LOGE("graph init failed\n");
        return -1;
    }

    for (int i = 0; i < 1; i++) {
        std::string img_path =
            "D:/Program/rayshape_deploy/images/other.png"; // 输入path写死,开始分析代码问题
        cv::Mat input_mat = cv::imread(img_path);
        input_edge.get()->SetMat(&input_mat, true); // 设置输入mat
        status = graph_demo.Run();
        if (status != RS_SUCCESS) {
            RS_LOGE("graph_demo run failed!\n");
            return -1;
        }
        // 输出边获取结果.
    }

    // 需要获取原始指针时使用 input_edge.get()
    // std::vector<dag::Edge *> inputs = {input_edge.get()};

    // dag::Edge *input_edge = new dag::Edge("input_edge");
    // std::vector<dag::Edge *> inputs = {input_edge};
    // std::vector<dag::Edge *> outputs = graph_demo.Trace(inputs); // 利用trace组件图graph.

    // get
    // for () {
    // }
    // run,运行
    // outputs = graph_demo(inputs);

    // dag::Edge *output_edge_ = outputs[0];

    // graph_demo.Deinit(); //主动deinit ,没有内容 图析构后自己deinit

    return 0;
}

int demo1(std::string img_path, std::string model_path, std::string inference_type) {
    // default construct the graph.
    rayshape::ClassificationGraph graph_default("graph_default");

    return 0;
}

int main(int argc, char **argv) {
    if (argc < 4) {
        std::cout << "Usage: " << argv[0]
                  << " <img_path/video_path> <serialized_model_path> <inference_type>" << std::endl;
        std::cout << "Note: model_path should be a serialized model file" << std::endl;
        return -1;
    }

    std::string img_path = argv[1];
    std::string model_path = argv[2];
    std::string inference_type = argv[3];

    int ret = PipelineScheduling(img_path, model_path, inference_type);

    return 0;
}

int PipelineScheduling(std::string img_path, std::string model_path, std::string inference_type) {
    // 理论上来说可以编造一个图片解码节点，这样就不用输入边在主线程中添加输入数据会方便很多.
    std::unique_ptr<dag::Edge> input_edge = std::make_unique<dag::Edge>("input_edge"); // 图的输入变

    std::unique_ptr<dag::Edge> output_edge = std::make_unique<dag::Edge>("output_edge");

    rayshape::ClassificationGraph graph_pipeline_demo("classification_pipeline_demo",
                                                      {input_edge.get()}, {output_edge.get()});

    ParallelType parall_type = PARALLEL_TYPE_PIPELINE;

    ErrorCode status = graph_pipeline_demo.SetParallelType(parall_type);
    if (status != RS_SUCCESS) {
        RS_LOGE("graph SetParallelType failed\n");
        return -1;
    }

    rayshape::InferenceType infer_type = InferenceType::OPENVINO;

    status = graph_pipeline_demo.MakeGraph(infer_type); // 构建图 #基本完成简单图的构建
    if (status != RS_SUCCESS) {
        RS_LOGE("graph MakeGraph failed\n");
        return -1;
    }

    status = graph_pipeline_demo.Init();
    if (status != RS_SUCCESS) {
        RS_LOGE("graph init failed\n");
        return -1;
    }

    // 视频路径
    cv::VideoCapture cap(img_path);
    if (!cap.isOpened()) {
        RS_LOGE("Cannot open video stream\n");
        return -1;
    }
    double fps = cap.get(cv::CAP_PROP_FPS);
    int width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    int height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    long totalFrames = cap.get(cv::CAP_PROP_FRAME_COUNT);
    std::cout << "FPS: " << fps << ", Resolution: " << width << "x" << height << std::endl;

    // cv::Mat frame; //考虑合成一个混合分类的视频
    while (true) {
        cv::Mat *input_frame = new cv::Mat(); // 主线程调度输入cv::Mat的数据.
        cap.read(*input_frame);

        input_edge.get()->SetMat(input_frame, false); // 设置输入mat
        status = graph_pipeline_demo.Run(); // pipeline流水线引擎所作的就是run_size++操作,没有其他.
        if (status != RS_SUCCESS) {
            RS_LOGE("graph_demo run failed!\n");
            return -1;
        }
    }

    cap.release();

    if (parall_type == PARALLEL_TYPE_PIPELINE) {
        // NNDEPLOY_LOGE("size = %d.\n", size);
        for (int i = 0; i < totalFrames; ++i) {
            Buffer *output_buffer = output_edge->GetGraphOutputBuffer();

            // 进行打印输出等
        }
    }

    return 0;
}