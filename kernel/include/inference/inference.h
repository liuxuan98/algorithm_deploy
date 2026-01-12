/**
 * @file inference.h
 * @brief 推理引擎管理的抽象模块
 * @copyright (c) 2025 Shenzhen RayShape Medical Technology Co., Ltd.
 *
 *
 * @author Liuxuan
 * @email liuxuan@rayshape.com
 * @date 2025-05-16
 * @version 1.0.0
 */

#ifndef INFERENCE_H
#define INFERENCE_H

#include "base/common.h"
#include "base/error.h"
#include "memory_manager/blob.h"
#include "model/model.h"

namespace rayshape
{
    namespace inference
    {

        /**
         * @brief 推理的基类
         * @details 初始化第三方的推理引擎(OpenVINO,TensorRT, NCNN, ONNXRUNTIME,
         * etc),统一多端推理引擎接口
         *
         */
        class RS_PUBLIC Inference {
        public:
            Inference(InferenceType type);

            virtual ~Inference();
            /**
             * @brief get inference engine type
             * @return InferenceType
             */
            InferenceType GetInferenceType();

            /**
             * @brief Inference Init for actually model
             * @param[in] model model information
             * @param[in] runtime inference runtime parameters
             * @return ErrorCode RS_SUCCESS if copy success, otherwise error code
             */
            virtual ErrorCode Init(const Model *model, const CustomRuntime *runtime) = 0;

            /**
             * @brief Inference resource free
             * @return void
             */
            virtual void DeInit() = 0;

            /**
             * @brief model inference engine forward infer
             * @return ErrorCode RS_SUCCESS if copy success, otherwise error code
             */
            virtual ErrorCode Forward() = 0;

            /**
             * @brief inference engine reshape for dynamic model
             * @param[in] name_arr need reshape model input name array
             * @param[in] dims_arr need reshape input shape array
             * @param[in] dims_size need reshape shape array size
             * @return ErrorCode RS_SUCCESS if copy success, otherwise error code
             */
            virtual ErrorCode Reshape(const char **name_arr, const Dims *dims_arr,
                                      size_t dims_size) = 0;

            /**
             * @brief get model all input blobs
             * @param[out] blob_arr input blobs array
             * @param[out] blob_size model input blob num
             * @return ErrorCode RS_SUCCESS if copy success, otherwise error code
             */
            virtual ErrorCode InputBlobsGet(const Blob ***blob_arr, size_t *blob_size) = 0;

            /**
             * @brief get model all output blobs
             * @param[out] blob_arr output blobs array
             * @param[out] blob_size model output blob num
             * @return ErrorCode RS_SUCCESS if copy success, otherwise error code
             */
            virtual ErrorCode OutputBlobsGet(const Blob ***blob_arr, size_t *blob_size) = 0;

            /**
             * @brief get single input blobs by name
             * @param[in] input_name model input blob name
             * @param[out] blob model input blob
             * @return ErrorCode RS_SUCCESS if copy success, otherwise error code
             */
            virtual ErrorCode InputBlobGet(const char *input_name, Blob **blob) = 0;

            /**
             * @brief get single output blobs by name
             * @param[in] input_name model output blob name
             * @param[out] blob model output blob
             * @return ErrorCode RS_SUCCESS if copy success, otherwise error code
             */
            virtual ErrorCode OutputBlobGet(const char *output_name, const Blob **blob) = 0;

        protected:
            /**
             * @brief inference engine type
             */
            InferenceType type_;
        };

        /**
         * @brief 推理框架的创建类,
         *
         */
        class InferenceCreator {
        public:
            virtual ~InferenceCreator(){};
            // virtual Inference *CreateInference(base::InferenceType type) = 0;
            virtual std::shared_ptr<Inference> CreateInference(InferenceType type) = 0;
        };

        /**
         * @brief 推理框架的创建类模板
         *
         * @tparam T
         */
        template <typename T> class TypeInferenceCreator: public InferenceCreator {
            std::shared_ptr<Inference> CreateInference(InferenceType type) override {
                return std::make_shared<T>(type);
            }
        };

        /**
         * @brief Get the Global Inference Creator Map object
         *
         * @return std::map<base::InferenceType, std::shared_ptr<InferenceCreator>>&
         */
        std::map<InferenceType, std::shared_ptr<InferenceCreator>> &GetGlobalInferenceCreatorMap();

        /**
         * @brief 推理框架的创建类的注册类模板
         *
         * @tparam T
         */
        template <typename T> class TypeInferenceRegister {
        public:
            explicit TypeInferenceRegister(InferenceType type) {
                GetGlobalInferenceCreatorMap()[type] = std::shared_ptr<T>(new T());
            }
        };

        /**
         * @brief Create a Inference instance object
         * @param[in] InferenceType type
         * @return std::shared_ptr<Inference>,Inference*
         */

        extern RS_PUBLIC std::shared_ptr<Inference> CreateInference(InferenceType type);

    } // namespace inference
} // namespace rayshape

#endif // INFERENCE_H
