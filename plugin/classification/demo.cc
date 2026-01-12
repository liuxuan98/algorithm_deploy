// #include "classification.h"
// #include "dag/edge.h"
// using namespace rayshape;
// int main(int argc, char **argv) {
//     std::cout << "begin build classificaiton graph!" << std::endl;
//     std::string graph_name = "Thyroid_Breast_classification";
//     dag::Edge graph_in("graph_in");
//     dag::Edge graph_out("graph_out");

//     rayshape::ClassificationGraph classification_graph(graph_name, {&graph_in}, {&graph_out});
//     InferenceType inference_type = InferenceType::OPENVINO; // use openvino to inference
//     classification_graph.MakeGraph(inference_type);

//     ErrorCode ret = classification_graph.Init(); // 初始化
//     if (ret != RS_SUCCESS) {
//         RS_LOGE("Grapg init failed!\n");
//         return ret;
//     }

//     classification_graph.Dump(); // 打印构建好的图结构

//     ret = classification_graph.Run(); // run process
//     if (ret != RS_SUCCESS) {
//         RS_LOGE("Grapg run failed!\n");
//         return ret;
//     }

//     return 0;
// }
