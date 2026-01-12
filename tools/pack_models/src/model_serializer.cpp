/**
 * @file model_serializer.cpp
 * @brief Model serialization utility implementation
 * @copyright (c) 2025 Shenzhen RayShape Medical Technology Co., Ltd.
 */

#include "model_serializer.h"
#include "../include/utils/file_utils.h"
#include "utils/codec/auto_crypto.h"

#include <fstream>
#include <sstream>
#include <iostream>

// Include model serialization registration - CRITICAL for cereal polymorphic serialization
#include "utils/codec/model_serialize.h"

#ifdef ENABLE_MNN_MODEL
#include "model/mnn/mnn_model.h"
#endif

#ifdef ENABLE_OPENVINO_MODEL
#include "model/openvino/openvino_model.h"
#endif

#ifdef ENABLE_ONNX_MODEL
#include "model/onnx/onnx_model.h"
#endif

namespace rayshape
{
    namespace tools
    {

        bool ModelSerializer::SerializeModelToFile(std::unique_ptr<Model> model,
                                                   const std::string &output_path,
                                                   bool auto_encrypt) {
            if (!model) {
                SetError("Model is null");
                return false;
            }

            try {
                // First serialize to buffer
                std::vector<uint8_t> buffer;
                if (!SerializeModelToBuffer(std::move(model), buffer, auto_encrypt)) {
                    return false;
                }

                // Write buffer to file
                if (!utils::FileUtils::WriteBinaryFile(output_path, buffer)) {
                    SetError("Failed to write serialized model to file: " + output_path);
                    return false;
                }

                return true;
            } catch (const std::exception &e) {
                SetError("Exception during model serialization: " + std::string(e.what()));
                return false;
            }
        }

        bool ModelSerializer::SerializeModelToBuffer(std::unique_ptr<Model> model,
                                                     std::vector<uint8_t> &buffer,
                                                     bool auto_encrypt) {
            if (!model) {
                SetError("Model is null");
                return false;
            }

            try {
                // Serialize model to binary data first
                std::ostringstream oss(std::ios::binary);
                ModelCodec model_codec(std::move(model));
                cereal::BinaryOutputArchive oa(oss);
                oa(model_codec);

                                std::string serialized_data = oss.str();
                std::vector<uint8_t> model_data(serialized_data.begin(), serialized_data.end());

                // If auto-encryption is enabled, encrypt the data
                if (auto_encrypt) {
                    if (!rayshape::utils::codec::AutoCrypto::IsAvailable()) {
                        SetError("Auto-encryption requested but not available");
                        return false;
                    }

                    if (!rayshape::utils::codec::AutoCrypto::AutoEncrypt(model_data, buffer)) {
                        SetError("Failed to auto-encrypt model data: " + rayshape::utils::codec::AutoCrypto::GetLastError());
                        return false;
                    }
                } else {
                    // No encryption, use raw serialized data
                    buffer = std::move(model_data);
                }

                return true;
            } catch (const std::exception &e) {
                SetError("Exception during model serialization to buffer: "
                         + std::string(e.what()));
                return false;
            }
        }

        bool ModelSerializer::SerializeMnnModel(const std::string &mnn_model_path,
                                                const std::string &config_path,
                                                const std::string &output_path,
                                                bool auto_encrypt) {
#ifdef ENABLE_MNN_MODEL
            if (!utils::FileUtils::FileExists(mnn_model_path)) {
                SetError("MNN model file does not exist: " + mnn_model_path);
                return false;
            }

            if (!ValidateModelFile(mnn_model_path, rayshape::ModelType::MNN)) {
                return false;
            }

            std::string model_content;
            if (!LoadFileContent(mnn_model_path, model_content)) {
                return false;
            }

            if (!utils::FileUtils::FileExists(config_path)) {
                SetError("Config file does not exist: " + config_path);
                return false;
            }

            std::string config_content;
            if (!LoadFileContent(config_path, config_content)) {
                return false;
            }

            try {
                auto mnn_model = std::make_unique<MNNModel>();
                mnn_model->bin_buf_ = model_content;
                mnn_model->cfg_str_ = config_content;
                return SerializeModelToFile(std::move(mnn_model), output_path, auto_encrypt);
            } catch (const std::exception &e) {
                SetError("Failed to create MNN model: " + std::string(e.what()));
                return false;
            }
#else
            (void)mnn_model_path;
            (void)config_path;
            (void)output_path;
            (void)auto_encrypt;
            SetError("MNN model support is not enabled in this build");
            return false;
#endif
        }

        bool ModelSerializer::SerializeOpenVINOModel(const std::string &xml_path,
                                                     const std::string &bin_path,
                                                     const std::string &config_path,
                                                     const std::string &output_path,
                                                     bool auto_encrypt) {
#ifdef ENABLE_OPENVINO_MODEL
            if (!utils::FileUtils::FileExists(xml_path)) {
                SetError("OpenVINO XML file does not exist: " + xml_path);
                return false;
            }

            if (!utils::FileUtils::FileExists(bin_path)) {
                SetError("OpenVINO BIN file does not exist: " + bin_path);
                return false;
            }

            if (!ValidateModelFile(xml_path, rayshape::ModelType::OPENVINO)) {
                return false;
            }

            std::string xml_content, bin_content;
            if (!LoadFileContent(xml_path, xml_content)
                || !LoadFileContent(bin_path, bin_content)) {
                return false;
            }

            if (!utils::FileUtils::FileExists(config_path)) {
                SetError("Config file does not exist: " + config_path);
                return false;
            }

            std::string config_content;
            if (!LoadFileContent(config_path, config_content)) {
                return false;
            }

            try {
                auto openvino_model = std::make_unique<OpenVINOModel>(xml_content, bin_content);
                openvino_model->cfg_str_ = config_content;
                return SerializeModelToFile(std::move(openvino_model), output_path, auto_encrypt);
            } catch (const std::exception &e) {
                SetError("Failed to create OpenVINO model: " + std::string(e.what()));
                return false;
            }
#else
            (void)xml_path;
            (void)bin_path;
            (void)config_path;
            (void)output_path;
            (void)auto_encrypt;
            SetError("OpenVINO model support is not enabled in this build");
            return false;
#endif
        }

        bool ModelSerializer::SerializeONNXModel(const std::string &onnx_model_path,
                                                 const std::string &config_path,
                                                 const std::string &output_path,
                                                 bool auto_encrypt) {
#ifdef ENABLE_ONNX_MODEL
            if (!utils::FileUtils::FileExists(onnx_model_path)) {
                SetError("ONNX model file does not exist: " + onnx_model_path);
                return false;
            }

            if (!ValidateModelFile(onnx_model_path, rayshape::ModelType::ONNX)) {
                return false;
            }

            std::string model_content;
            if (!LoadFileContent(onnx_model_path, model_content)) {
                return false;
            }

            if (!utils::FileUtils::FileExists(config_path)) {
                SetError("Config file does not exist: " + config_path);
                return false;
            }

            std::string config_content;
            if (!LoadFileContent(config_path, config_content)) {
                return false;
            }

            try {
                auto onnx_model = std::make_unique<ONNXModel>();
                onnx_model->bin_buf_ = model_content;
                onnx_model->cfg_str_ = config_content;
                return SerializeModelToFile(std::move(onnx_model), output_path, auto_encrypt);
            } catch (const std::exception &e) {
                SetError("Failed to create ONNX model: " + std::string(e.what()));
                return false;
            }
#else
            (void)onnx_model_path;
            (void)config_path;
            (void)output_path;
            (void)auto_encrypt;
            SetError("ONNX model support is not enabled in this build");
            return false;
#endif
        }

        bool ModelSerializer::ValidateModelFile(const std::string &model_path,
                                                rayshape::ModelType model_type) {
            if (!utils::FileUtils::FileExists(model_path)) {
                SetError("Model file does not exist: " + model_path);
                return false;
            }

            long long file_size = utils::FileUtils::GetFileSize(model_path);
            if (file_size <= 0) {
                SetError("Invalid model file size: " + model_path);
                return false;
            }

            // Basic file extension validation
            std::string extension = utils::FileUtils::GetFileExtension(model_path);
            switch (model_type) {
            case rayshape::ModelType::MNN:
                if (extension != "mnn") {
                    SetError("Expected .mnn file extension for MNN model");
                    return false;
                }
                break;
            case rayshape::ModelType::OPENVINO:
                if (extension != "xml" && extension != "bin") {
                    SetError("Expected .xml or .bin file extension for OpenVINO model");
                    return false;
                }
                break;
            case rayshape::ModelType::ONNX:
                if (extension != "onnx") {
                    SetError("Expected .onnx file extension for ONNX model");
                    return false;
                }
                break;
            default:
                // No specific validation for other types
                break;
            }

            return true;
        }

        bool ModelSerializer::LoadFileContent(const std::string &file_path, std::string &content) {
            if (!utils::FileUtils::ReadFileContent(file_path, content)) {
                SetError("Failed to read file content: " + file_path);
                return false;
            }
            return true;
        }

        void ModelSerializer::SetError(const std::string &error) {
            last_error_ = error;
            std::cerr << "ModelSerializer Error: " << error << std::endl;
        }

    } // namespace tools
} // namespace rayshape
