#ifndef _INFERENCE_H_
#define _INFERENCE_H_

#include "base/common.h"
#include "base/error.h"
#include "memory_manager/blob.h"
#include "utils/model.h"

namespace rayshape
{
    namespace inference
    {

        /**
         * @brief 推理的基类
         * @details
         * # 根据Model工具类解析模型文件,解析json的基本信息,来初始化第三方的推理引擎(OpenVINO, TensorRT, NCNN, ONNXRUNTIME, etc)
         * # 得到输入blob/tensor信息和数据
         * # 推理
         * # 得到输出blob/tensor信息和数据
         * # 动态Reshape根据动态的输入来Reshape推理引擎
         *
         */
        class RS_PUBLIC Inference
        {
        public:
            Inference(InferenceType type);
            virtual ~Inference(); // 虚析构

            InferenceType GetInferenceType();

            /**
             * @brief Inference Init
             *
             * @return ErrorCode
             */
            virtual ErrorCode Init(const Model *model, const CustomRuntime *runtime) = 0;

            /**
            * @brief Inference Init for openvino test
            * //重载一个独特的init接口给openvino网络使用
            * @return ErrorCode
            */
            virtual ErrorCode Init(const std::string& xml_path, const std::string& bin_path) = 0;

            /**
             * @brief Inference Deinit
             *
             * @return ErrorCode
             */
            virtual void DeInit() = 0;

            /**
             * @brief Inference Infer
             *
             * @return ErrorCode
             */
            virtual ErrorCode Forward() = 0;

            /**
             * @brief 针对动态输入的推理，设置输入tensor的shape
             *
             * @param shape_map
             * @return ErrorCode
             */
            virtual ErrorCode Reshape(const char **name_arr, const Dims *dims_arr, size_t dims_size) = 0;

            // @brief get all input blobs
            // @param blobs input blobs name map
            virtual ErrorCode InputBlobsGet(const Blob ***blob_arr, size_t *blob_size) = 0;

            // @brief get all output blobs
            // @param blobs output blobs name map
            virtual ErrorCode OutputBlobsGet(const Blob ***blob_arr, size_t *blob_size) = 0;

            // @brief get input output blob by input_name
            // @param blobs output blobs name map
            virtual ErrorCode InputBlobGet(const char *input_name, Blob **blob) = 0;

            virtual ErrorCode OutputBlobGet(const char *output_name, const Blob **blob) = 0;

        protected:
            /**
             * @brief 推理框架的类型
             */
            InferenceType type_;
        };

        /**
         * @brief 推理框架的创建类,
         *
         */
        class InferenceCreator
        {
        public:
            virtual ~InferenceCreator(){};
            // virtual Inference *CreateInference(base::InferenceType type) = 0;
            virtual std::shared_ptr<Inference> CreateInference(
                InferenceType type) = 0;
        };

        /**
         * @brief 推理框架的创建类模板
         *
         * @tparam T
         */
        template <typename T>
        class TypeInferenceCreator : public InferenceCreator
        {
            // virtual Inference *CreateInference(base::InferenceType type) {
            //   return new T(type);
            // }
            virtual std::shared_ptr<Inference> CreateInference(InferenceType type)
            {
                return std::make_shared<T>(type);
            }
        };

        /**
         * @brief Get the Global Inference Creator Map object
         *
         * @return std::map<base::InferenceType, std::shared_ptr<InferenceCreator>>&
         */
        std::map<InferenceType, std::shared_ptr<InferenceCreator>> &
        GetGlobalInferenceCreatorMap();

        /**
         * @brief 推理框架的创建类的注册类模板
         *
         * @tparam T
         */
        template <typename T>
        class TypeInferenceRegister
        {
        public:
            explicit TypeInferenceRegister(InferenceType type)
            {
                GetGlobalInferenceCreatorMap()[type] = std::shared_ptr<T>(new T());
            }
        };

        /**
         * @brief Create a Inference object
         *
         * @param[in] InferenceType type
         * @return Inference*
         */

        extern RS_PUBLIC std::shared_ptr<Inference> CreateInference(
            InferenceType type);

    } // namespace inference
} // namespace rayshape

#endif