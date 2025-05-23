#include "inference/openvino/openvino_blob_converter.h"
#include "inference/openvino/openvino_config_converter.h"
#include "utils/memory_size_info.h"
#include "utils/blob_utils.h"

using namespace rayshape::utils;

namespace rayshape
{
    namespace openvino
    {

        std::shared_ptr<ov::Tensor> OpenvinoBlobConverter::ConvertFromBlob(ErrorCode &status, const Blob *src)
        {
            if (src == nullptr)
            {
                status = RS_NULL_PARAM;
                return nullptr;
            }

            status = RS_INVALID_MODEL;

            ov::element::Type openvino_precision;
            status = OpenVINOConfigConverter::ConvertFromDataType(openvino_precision, src->data_type);
            if (status != RS_SUCCESS)
            {
                // log
                return nullptr;
            }

            ov::Layout openvino_layout;
            status = OpenVINOConfigConverter::ConvertFromDataFormat(openvino_layout, src->data_format);
            if (status != RS_SUCCESS)
            {
                // log
                return nullptr;
            }
            ov::Shape openvino_dims;
            status = OpenVINOConfigConverter::ConvertFromDims(openvino_dims, src->dims);
            if (status != RS_SUCCESS)
            {
                // log
                return nullptr;
            }

            std::shared_ptr<ov::Tensor> dst_tensor = nullptr;
            char *data = (char *)(src->buffer)->GetSrcData();

            if (ov::element::f32 == openvino_precision)
            {
                dst_tensor.reset(new ov::Tensor(openvino_precision, openvino_dims, (float *)data));
            }
            else if (ov::element::f16 == openvino_precision)
            {
                dst_tensor.reset(new ov::Tensor(openvino_precision, openvino_dims, (char16_t *)data));
            }
            else if (ov::element::u8 == openvino_precision)
            {
                dst_tensor.reset(new ov::Tensor(openvino_precision, openvino_dims, (uint8_t *)data));
            }
            else if (ov::element::i32 == openvino_precision)
            {
                dst_tensor.reset(new ov::Tensor(openvino_precision, openvino_dims, (int32_t *)data));
            }
            else if (ov::element::i64 == openvino_precision)
            {
                dst_tensor.reset(new ov::Tensor(openvino_precision, openvino_dims, (int64_t *)data));
            }
            else if (ov::element::u32 == openvino_precision)
            {
                dst_tensor.reset(new ov::Tensor(openvino_precision, openvino_dims, (uint32_t *)data));
            }
            else
            {
                status = RS_INVALID_MODEL;
                // log
                dst_tensor = nullptr;
            }

            return dst_tensor;
        }

        ErrorCode OpenvinoBlobConverter::CreateOrUpdateBlob(Blob **dst, const ov::Tensor &src, const char *blob_name, bool alloc, bool gpu_blob)
        {
            if (dst == nullptr)
            {
                return RS_INVALID_PARAM;
            }
            ErrorCode ret = RS_SUCCESS;

            if (*dst != nullptr)
            {
                free(*dst); // 防止内存泄漏
                *dst = nullptr;
            }

            Blob *blob = (Blob *)malloc(sizeof(Blob));
            if (blob == nullptr)
            {
                return RS_OUTOFMEMORY;
            }
            memset(blob, 0, sizeof(Blob));
            //   convert data type
            blob->data_type = OpenVINOConfigConverter::ConvertToDataType(src.get_element_type());

            // convert shape dims
            ret = OpenVINOConfigConverter::ConvertToDims(blob->dims, src.get_shape());
            if (ret != RS_SUCCESS)
            {
                return ret;
            }

            // convert data layout
            std::string layout = GetDataLayoutString(blob->dims);
            blob->data_format = OpenVINOConfigConverter::ConvertToDataFormat(layout);

            // device type
            if (gpu_blob)
            {
                blob->device_type = DEVICE_TYPE_INTERL_GPU;
            }
            else
            {
                blob->device_type = DEVICE_TYPE_CPU;
            }
            // blob name
            strncpy(blob->name, blob_name, strlen(blob_name));

            if (alloc)
            {
                size_t byte_size = CalculateMemorySize(blob->dims, blob->data_type);

                blob->buffer = new Buffer(byte_size, blob->device_type, false);
            }
            else
            {
                void *data = src.data(); // 获得浅拷贝指针
                size_t size = src.get_byte_size();
                blob->buffer = new Buffer(size, blob->device_type, true, data);
                //
            }

            *dst = blob;
            return RS_SUCCESS;
        }

    }
} // namespace rayshape::openvino