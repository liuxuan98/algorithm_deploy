#include "inference/openvino/openvino_blob_converter.h"
#include "inference/openvino/openvino_config_converter.h"
#include "utils/memory_size_info.h"
#include "utils/blob_utils.h"
#include "base/logger.h"
#include "utils/device_convert_utils.h"

using namespace rayshape::utils;

namespace rayshape
{
    namespace inference
    {
        namespace openvino
        {

            std::shared_ptr<ov::Tensor> OpenvinoBlobConverter::ConvertFromBlob(ErrorCode &status,
                                                                               const Blob *src) {
                if (src == nullptr) {
                    status = RS_NULL_PARAM;
                    return nullptr;
                }

                status = RS_INVALID_MODEL;

                ov::element::Type openvino_precision;
                status = OpenVINOConfigConverter::ConvertFromDataType(openvino_precision,
                                                                      src->data_type);
                if (status != RS_SUCCESS) {
                    RS_LOGE("OpenvinoBlobConverter::ConvertFromDataType failed:%d.\n", status);
                    return nullptr;
                }

                ov::Layout openvino_layout;
                status = OpenVINOConfigConverter::ConvertFromDataFormat(openvino_layout,
                                                                        src->data_format);
                if (status != RS_SUCCESS) {
                    RS_LOGE("OpenvinoBlobConverter::ConvertFromDataFormat failed:%d.\n", status);
                    return nullptr;
                }
                ov::Shape openvino_dims;
                status = OpenVINOConfigConverter::ConvertFromDims(openvino_dims, src->dims);
                if (status != RS_SUCCESS) {
                    RS_LOGE("OpenvinoBlobConverter::ConvertFromDims failed:%d.\n", status);
                    return nullptr;
                }

                std::shared_ptr<ov::Tensor> dst_tensor = nullptr;
                char *data = (char *)(src->buffer)->GetDataPtr();

                if (ov::element::f32 == openvino_precision) {
                    dst_tensor.reset(
                        new ov::Tensor(openvino_precision, openvino_dims, (float *)data));
                } else if (ov::element::f16 == openvino_precision) {
                    dst_tensor.reset(
                        new ov::Tensor(openvino_precision, openvino_dims, (char16_t *)data));
                } else if (ov::element::u8 == openvino_precision) {
                    dst_tensor.reset(
                        new ov::Tensor(openvino_precision, openvino_dims, (uint8_t *)data));
                } else if (ov::element::i32 == openvino_precision) {
                    dst_tensor.reset(
                        new ov::Tensor(openvino_precision, openvino_dims, (int32_t *)data));
                } else if (ov::element::i64 == openvino_precision) {
                    dst_tensor.reset(
                        new ov::Tensor(openvino_precision, openvino_dims, (int64_t *)data));
                } else if (ov::element::u32 == openvino_precision) {
                    dst_tensor.reset(
                        new ov::Tensor(openvino_precision, openvino_dims, (uint32_t *)data));
                } else {
                    status = RS_INVALID_MODEL;
                    RS_LOGE("not yet support ov::Tensor precision\n");
                    dst_tensor = nullptr;
                }

                return dst_tensor;
            }

            ErrorCode OpenvinoBlobConverter::CreateOrUpdateBlob(Blob **dst, const ov::Tensor &src,
                                                                const char *blob_name, bool alloc,
                                                                bool gpu_blob) {
                if (dst == nullptr || blob_name == nullptr) {
                    RS_LOGE("input params dst or blob_name is null\n");
                    return RS_INVALID_PARAM;
                }
                ErrorCode ret = RS_SUCCESS;

                if (*dst != nullptr) {
                    free(*dst); // 防止内存泄漏
                    *dst = nullptr;
                }

                Blob *blob = (Blob *)malloc(sizeof(Blob));
                if (blob == nullptr) {
                    RS_LOGE("blob malloc Blob:%zu failed.\n", sizeof(Blob));
                    return RS_OUTOFMEMORY;
                }
                memset(blob, 0, sizeof(Blob));
                //   convert data type
                blob->data_type =
                    OpenVINOConfigConverter::ConvertToDataType(src.get_element_type());

                // convert shape dims
                ret = OpenVINOConfigConverter::ConvertToDims(blob->dims, src.get_shape());
                if (ret != RS_SUCCESS) {
                    RS_LOGE("OpenVINOConfigConverter::ConvertToDims failed:%d.\n", ret);
                    return ret;
                }

                // convert data layout
                std::string layout = GetDataLayoutString(blob->dims);
                blob->data_format = OpenVINOConfigConverter::ConvertToDataFormat(layout);

                // device type
                if (gpu_blob) {
                    blob->device_type = DeviceType::INTERL_GPU;
                } else {
                    blob->device_type = DeviceType::CPU;
                }
                // blob name
                strncpy(blob->name, blob_name, strlen(blob_name));

                MemoryType mem_type = MemoryType::NONE;
                ret = ConvertDeviceTypeToMemory(blob->device_type, mem_type);
                if (ret != RS_SUCCESS) {
                    RS_LOGE("ConvertDeviceTypeToMemory failed\n");
                    delete blob;
                    return ret;
                }

                if (alloc) {
                    size_t size = CalculateDims(blob->dims); // src.get_byte_size();
                    RSMemoryInfo mem_info{mem_type, blob->data_type,
                                          static_cast<unsigned int>(size)};
                    blob->buffer = Buffer::Alloc(mem_info);
                    if (blob->buffer == nullptr) {
                        RS_LOGE("Buffer Alloc failed\n");
                        return RS_OUTOFMEMORY;
                    }
                } else {
                    void *data = src.data(); // 获得浅拷贝指针
                    size_t size = src.get_byte_size();
                    RSMemoryInfo mem_info{mem_type, blob->data_type,
                                          static_cast<unsigned int>(size)};
                    blob->buffer = Buffer::Create(data, mem_info);
                    if (blob->buffer == nullptr) {
                        RS_LOGE("Buffer Alloc failed\n");
                        return RS_OUTOFMEMORY;
                    }
                }

                *dst = blob;
                return RS_SUCCESS;
            }

        } // namespace openvino
    }     // namespace inference
} // namespace rayshape
