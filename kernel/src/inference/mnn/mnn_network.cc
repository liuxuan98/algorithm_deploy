#include "inference/mnn/mnn_network.h"
#include "inference/mnn/mnn_blob_converter.h"
#include "inference/mnn/mnn_config_converter.h"
#include "model/mnn/mnn_model.h"
#include "utils/blob_utils.h"
#include "base/logger.h"

using namespace rayshape::mnn;
using namespace rayshape::utils;

namespace rayshape
{
    namespace inference
    {
        // Register MNN inference type
        TypeInferenceRegister<TypeInferenceCreator<MNNNetwork>> g_mnn_inference_register(
            InferenceType::MNN);

        MNNNetwork::MNNNetwork(InferenceType type) : Inference(type) {}

        MNNNetwork::~MNNNetwork() {
            ClearBlobArray();
        }

        ErrorCode MNNNetwork::Init(const Model *model, const CustomRuntime *runtime) {
            RS_LOGD("MNNNetWork Init!\n");

            ErrorCode ret = RS_SUCCESS;

            if (model == nullptr || runtime == nullptr) {
                RS_LOGE("model or runtime is nullptr!\n");
                return RS_INVALID_PARAM;
            }

            device_type_ = runtime->device_type_;
            num_threads_ = runtime->num_thread_;

            auto mnn_model = dynamic_cast<const MNNModel *>(model);
            if (mnn_model == nullptr) {
                RS_LOGE("MNNModel is nullptr!\n");
                return RS_INVALID_MODEL;
            }

            RSJsonHandle json_handle = nullptr;
            do {
                if ((ret = RSJsonCreate(mnn_model->cfg_str_.c_str(), &json_handle)) != RS_SUCCESS) {
                    RS_LOGE("RSJsonCreate failed:%d\n", ret);
                    break;
                }

                if ((ret = ParseInputShapes(json_handle)) != RS_SUCCESS) {
                    RS_LOGE("ParseInputShapes failed:%d!\n", ret);
                    break;
                }

                if ((ret = InitWithModel(mnn_model->bin_buf_)) != RS_SUCCESS) {
                    RS_LOGE("InitWithModel failed:%d!\n", ret);
                    break;
                }

                if ((ret = Reshape()) != RS_SUCCESS) {
                    RS_LOGE("Reshape failed:%d!\n", ret);
                    break;
                }

                if ((ret = CreateBlobArray()) != RS_SUCCESS) {
                    RS_LOGE("CreateBlobArray failed:%d!\n", ret);
                    break;
                }
            } while (false);

            // Clean up resources
            if (json_handle != nullptr) {
                RSJsonDestory(&json_handle);
            }

            return ret;
        }

        void MNNNetwork::DeInit() {
            // TODO: Implement resource cleanup
            ClearBlobArray();

            if (session_ != nullptr) {
                interpreter_->releaseSession(session_);
                session_ = nullptr;
            }

            interpreter_.reset();
        }

        ErrorCode MNNNetwork::InitWithModel(const std::string &model_buf) {
            ErrorCode ret = RS_SUCCESS;

            try {
                // Create MNN interpreter
                interpreter_ = std::shared_ptr<MNN::Interpreter>(
                    MNN::Interpreter::createFromBuffer(model_buf.c_str(), model_buf.size()));
                if (interpreter_ == nullptr) {
                    RS_LOGE("Failed to create MNN interpreter from buffer\n");
                    return RS_MODEL_ERROR;
                }

                // Configure backend
                MNN::BackendConfig backend_config;
                MNNForwardType forward_type;
                ret = MNNConfigConverter::ConvertFromDevice(device_type_, backend_config,
                                                            forward_type);
                if (ret != RS_SUCCESS) {
                    RS_LOGE("ConvertFromDevice failed:%d\n", ret);
                    return ret;
                }

                // Setup schedule config
                schedule_config_.type = forward_type;
                schedule_config_.numThread = num_threads_;
                schedule_config_.backendConfig = &backend_config;

                // Create session
                session_ = interpreter_->createSession(schedule_config_);
                if (session_ == nullptr) {
                    RS_LOGE("Failed to create MNN session\n");
                    return RS_MODEL_ERROR;
                }

            } catch (const std::exception &e) {
                RS_LOGE("MNN model initialization failed: %s\n", e.what());
                return RS_MODEL_ERROR;
            }

            return ret;
        }

        ErrorCode MNNNetwork::ParseInputShapes(const RSJsonHandle json_handle) {
            // TODO: Implement input shape parsing from JSON
            ErrorCode ret = RS_SUCCESS;

            RSJsonObject root_obj = RSJsonRootGet(json_handle);
            RSJsonObject model_obj = RSJsonObjectGet(root_obj, "ModelConfig");
            if (model_obj == nullptr) {
                RS_LOGE("key ModelConfig's value is empty!\n");
                return RS_INVALID_PARAM;
            }

            RSJsonObject max_shapes_arr = RSJsonObjectGet(model_obj, "MaxShapes");
            if (max_shapes_arr == nullptr) {
                return RS_SUCCESS;
            }

            unsigned int size = RSJsonArraySize(max_shapes_arr);
            for (unsigned int i = 0; i < size; i++) {
                RSJsonObject shape_obj = RSJsonArrayAt(max_shapes_arr, i);
                RSJsonObject name_obj = RSJsonObjectGet(shape_obj, "name");
                const char *name = RSJsonStringGet(name_obj);

                if (name == nullptr || strlen(name) <= 0 || strlen(name) > MAX_BLOB_NAME) {
                    RS_LOGE("InputMaxShape Name is empty or > MAX_BLOB_NAME:%d!\n", MAX_BLOB_NAME);
                    return RS_INVALID_MODEL;
                }

                RSJsonObject dims_arr = RSJsonObjectGet(shape_obj, "dims");
                if (dims_arr == nullptr) {
                    RS_LOGE("InputMaxShape Dims is Error!\n");
                    return RS_INVALID_MODEL;
                }

                unsigned int dims_size = RSJsonArraySize(dims_arr);
                if (dims_size <= 0 || dims_size > MAX_DIMS_SIZE) {
                    RS_LOGE("InputMaxShape Dims size:%d is empty or > MAX_DIMS_SIZE:%d!\n",
                            dims_size, MAX_DIMS_SIZE);
                    return RS_INVALID_MODEL;
                }

                Dims dims;
                dims.size = dims_size;
                for (unsigned int j = 0; j < dims_size; j++) {
                    dims.value[j] = RSJsonIntGet(RSJsonArrayAt(dims_arr, j), 0);
                }
                input_max_shapes_[name] = dims;
            }

            return RS_SUCCESS;
        }

        ErrorCode MNNNetwork::Reshape(const char **name_arr, const Dims *dims_arr,
                                      size_t dims_size) {
            // TODO: Implement dynamic reshape
            ErrorCode ret = RS_SUCCESS;

            if (name_arr == nullptr || dims_arr == nullptr) {
                RS_LOGE("Invalid input parameters for Reshape.\n");
                return RS_INVALID_PARAM;
            }

            // MNN reshape implementation would go here
            RS_LOGD("MNNNetwork::Reshape not fully implemented yet\n");
            return RS_NOT_IMPLEMENT;
        }

        ErrorCode MNNNetwork::Reshape() {
            // TODO: Implement reshape with predefined shapes
            ErrorCode ret = RS_SUCCESS;

            if (input_max_shapes_.size() > 0) {
                // Reshape based on max shapes
                for (const auto &it : input_max_shapes_) {
                    const std::string &name = it.first;
                    const Dims &dims = it.second;

                    // MNN reshape implementation would go here
                    RS_LOGD("Reshaping input %s\n", name.c_str());
                }
            }

            return ret;
        }

        ErrorCode MNNNetwork::CreateBlobArray() {
            // TODO: Implement blob array creation
            ErrorCode ret = RS_SUCCESS;
            ClearBlobArray();

            if (interpreter_ == nullptr || session_ == nullptr) {
                RS_LOGE("MNN interpreter or session is null\n");
                return RS_INVALID_MODEL;
            }

            // Get input tensors
            auto input_names = interpreter_->getSessionInputAll(session_);
            size_t input_count = input_names.size();

            if (input_count <= 0) {
                RS_LOGE("MNN model inputs count:%zu error.\n", input_count);
                return RS_INVALID_MODEL;
            }

            input_blob_arr_ = (Blob **)malloc(sizeof(Blob *) * input_count);
            if (input_blob_arr_ == nullptr) {
                RS_LOGE("input blobs malloc failed\n");
                return RS_OUTOFMEMORY;
            }
            memset(input_blob_arr_, 0, sizeof(Blob *) * input_count);

            // Create input blobs
            for (const auto &input_pair : input_names) {
                const std::string &name = input_pair.first;
                MNN::Tensor *tensor = input_pair.second;

                ret = MNNBlobConverter::CreateOrUpdateBlob(&input_blob_arr_[input_blob_size_++],
                                                           tensor, name.c_str(), false);
                if (ret != RS_SUCCESS) {
                    RS_LOGE("MNNBlobConverter::CreateOrUpdateBlob failed:%d!\n", ret);
                    ClearBlobArray();
                    return ret;
                }

                input_tensors_.push_back(tensor);
            }

            // Get output tensors
            auto output_names = interpreter_->getSessionOutputAll(session_);
            size_t output_count = output_names.size();

            if (output_count <= 0) {
                RS_LOGE("MNN model outputs count:%zu error.\n", output_count);
                ClearBlobArray();
                return RS_INVALID_MODEL;
            }

            output_blob_arr_ = (Blob **)malloc(sizeof(Blob *) * output_count);
            if (output_blob_arr_ == nullptr) {
                RS_LOGE("output blobs malloc failed\n");
                ClearBlobArray();
                return RS_OUTOFMEMORY;
            }
            memset(output_blob_arr_, 0, sizeof(Blob *) * output_count);

            // Create output blobs
            for (const auto &output_pair : output_names) {
                const std::string &name = output_pair.first;
                MNN::Tensor *tensor = output_pair.second;

                ret = MNNBlobConverter::CreateOrUpdateBlob(&output_blob_arr_[output_blob_size_++],
                                                           tensor, name.c_str(), false);
                if (ret != RS_SUCCESS) {
                    RS_LOGE("MNNBlobConverter::CreateOrUpdateBlob failed:%d!\n", ret);
                    ClearBlobArray();
                    return ret;
                }

                output_tensors_.push_back(tensor);
            }

            return ret;
        }

        void MNNNetwork::ClearBlobArray() {
            // TODO: Implement blob array cleanup
            if (input_blob_arr_ != nullptr && input_blob_size_ != 0) {
                for (size_t i = 0; i < input_blob_size_; ++i) {
                    if (input_blob_arr_[i]) {
                        BlobFree(input_blob_arr_[i]);
                    }
                }
                free(input_blob_arr_);
                input_blob_arr_ = nullptr;
                input_blob_size_ = 0;
            }

            if (output_blob_arr_ != nullptr && output_blob_size_ != 0) {
                for (size_t i = 0; i < output_blob_size_; ++i) {
                    if (output_blob_arr_[i]) {
                        BlobFree(output_blob_arr_[i]);
                    }
                }
                free(output_blob_arr_);
                output_blob_arr_ = nullptr;
                output_blob_size_ = 0;
            }

            input_tensors_.clear();
            output_tensors_.clear();
        }

        ErrorCode MNNNetwork::Forward() {
            // TODO: Implement forward inference
            ErrorCode ret = RS_SUCCESS;

            if (interpreter_ == nullptr || session_ == nullptr) {
                RS_LOGE("MNN interpreter or session is null\n");
                return RS_INVALID_MODEL;
            }

            try {
                // Set input tensors
                for (size_t i = 0; i < input_blob_size_; ++i) {
                    Blob *blob = input_blob_arr_[i];
                    Buffer *blob_buffer = blob->buffer;
                    bool need_set_input = blob_buffer->GetExternalFlag();

                    if (!need_set_input) {
                        const char *name = blob->name;
                        std::shared_ptr<MNN::Tensor> mnn_tensor =
                            MNNBlobConverter::ConvertFromBlob(ret, blob);
                        if (mnn_tensor == nullptr || ret != RS_SUCCESS) {
                            RS_LOGE("ConvertFromBlob failed:%d\n", ret);
                            return ret;
                        }

                        // Copy data to input tensor
                        MNN::Tensor *input_tensor = interpreter_->getSessionInput(session_, name);
                        if (input_tensor != nullptr) {
                            input_tensor->copyFromHostTensor(mnn_tensor.get());
                        }
                    }
                }

                // Run inference
                MNN::ErrorCode mnn_ret = interpreter_->runSession(session_);
                if (mnn_ret != MNN::NO_ERROR) {
                    RS_LOGE("MNN runSession failed with code:%d\n", mnn_ret);
                    return RS_MODEL_ERROR;
                }

            } catch (const std::exception &e) {
                RS_LOGE("MNN model inference failed: %s\n", e.what());
                return RS_MODEL_ERROR;
            }

            return RS_SUCCESS;
        }

        ErrorCode MNNNetwork::InputBlobsGet(const Blob ***blob_arr, size_t *blob_size) {
            if (blob_arr == nullptr || blob_size == nullptr) {
                RS_LOGE("blob_arr:%p or blob_size:%p is nullptr\n", blob_arr, blob_size);
                return RS_INVALID_PARAM;
            }

            if (input_blob_arr_ == nullptr || input_blob_size_ <= 0) {
                RS_LOGE("input_blob_arr_:%p is nullptr or input_blob_size_:%zu <= 0\n",
                        input_blob_arr_, input_blob_size_);
                return RS_INVALID_MODEL;
            }

            *blob_arr = (const Blob **)input_blob_arr_;
            *blob_size = input_blob_size_;

            return RS_SUCCESS;
        }

        ErrorCode MNNNetwork::OutputBlobsGet(const Blob ***blob_arr, size_t *blob_size) {
            if (blob_arr == nullptr || blob_size == nullptr) {
                RS_LOGE("blob_arr:%p or blob_size:%p is nullptr\n", blob_arr, blob_size);
                return RS_INVALID_PARAM;
            }

            if (output_blob_arr_ == nullptr || output_blob_size_ <= 0) {
                RS_LOGE("output_blob_arr_:%p is nullptr or output_blob_size_:%zu <= 0\n",
                        output_blob_arr_, output_blob_size_);
                return RS_INVALID_MODEL;
            }

            *blob_arr = (const Blob **)output_blob_arr_;
            *blob_size = output_blob_size_;

            return RS_SUCCESS;
        }

        ErrorCode MNNNetwork::InputBlobGet(const char *input_name, Blob **blob) {
            if (input_blob_arr_ == nullptr || blob == nullptr || input_blob_size_ <= 0) {
                RS_LOGE("blob:%p input_blob_arr_ is nullptr or input_blob_size_:%zu is empty\n",
                        blob, input_blob_size_);
                return RS_INVALID_MODEL;
            }

            int index = -1;
            *blob = FindBlobAndIndexByName(input_blob_arr_, input_blob_size_, input_name, &index);
            if (*blob == nullptr) {
                RS_LOGE("Not find Blob name:%s\n", input_name != nullptr ? input_name : "");
                return RS_INVALID_PARAM;
            }

            return RS_SUCCESS;
        }

        ErrorCode MNNNetwork::OutputBlobGet(const char *output_name, const Blob **blob) {
            if (output_blob_arr_ == nullptr || blob == nullptr || output_blob_size_ <= 0) {
                RS_LOGE("blob:%p output_blob_arr_ is nullptr or output_blob_size_:%zu is empty\n",
                        blob, output_blob_size_);
                return RS_INVALID_MODEL;
            }

            int index = -1;
            Blob *find_blob =
                FindBlobAndIndexByName(output_blob_arr_, output_blob_size_, output_name, &index);
            if (find_blob == nullptr) {
                RS_LOGE("Not find Blob name:%s\n", output_name != nullptr ? output_name : "");
                return RS_INVALID_PARAM;
            }

            *blob = find_blob;

            return RS_SUCCESS;
        }

    } // namespace inference
} // namespace rayshape
