#include "inference/openvino/openvino_blob_converter.h"
#include "inference/openvino/openvino_config_converter.h"


namespace rayshape
{
    namespace openvino
    {

        std::shared_ptr<ov::Tensor> OpenvinoBlobConverter::ConvertFromBlob(ErrorCode &status, const Blob *src)
        {
            if (src == nullptr)
            {
                status = RS_INVALID_PARAM_VALUE;
                return nullptr;
            }

            status = RS_INVALID_MODEL;

            ov::element::Type openvino_precision;
            status = OpenVINOConfigConverter::ConvertFromDataType(openvino_precision, src->data_type);
            if (status != RS_SUCCESS)
            {
                // 打印日志
                return nullptr;
            }

            ov::Layout openvino_layout;
            status = OpenVINOConfigConverter::ConvertFromDataFormat(openvino_layout, src->data_format);
            if (status != RS_SUCCESS)
            {
                // 打印日志
                return nullptr;
            }
            ov::Shape openvino_dims;
            status = OpenVINOConfigConverter::ConvertFromDims(openvino_dims, src->dims);
            if (status != RS_SUCCESS)
            {
                // 打印日志
                return nullptr;
            }

            std::shared_ptr<ov::Tensor> dst_tensor = nullptr;
            char *data = (char *)(src->buffer)->GetSrcData();

            if (ov::element::f32 == openvino_precision)
            {
                dst_tensor.reset(new ov::Tensor(openvino_precision, openvino_dims, (float *)data));
            }
            else if (ov::element::u16 == openvino_precision)
            {
                // todo
            }
            else
            {
                status = RS_INVALID_MODEL;
                // 打印日志
                dst_tensor = nullptr;
                // return nullptr;
            }

            return dst_tensor;
        }

        ErrorCode OpenvinoBlobConverter::CreateOrUpdateBlob(Blob **dst, const ov::Tensor &src, const char *blob_name, bool alloc)
        {
            ErrorCode ret = RS_SUCCESS;
            //if ((*dst)->name != blob_name) //对比
            //{
            //    //
            //    return RS_INVALID_PARAM;
            //}
            // convert data type
            DataType data_type = OpenVINOConfigConverter::ConvertToDataType(src.get_element_type());
            (*dst)->data_type = data_type;

            // convert input format(layout)
           /* ov::Output<const ov::Node> ov_input = model_->input(std::string(blob_name));
            ov::Layout input_layout = ov::layout::get_layout(ov_input);*/
            std::string layout = "NCHW"; //为了编译暂时固定排布
            //input_layout.to_string();
            DataFormat data_format = OpenVINOConfigConverter::ConvertToDataFormat(layout);

            // convert shape dims
            ret = OpenVINOConfigConverter::ConvertToDims((*dst)->dims, src.get_shape());
            if (ret != RS_SUCCESS)
            {
                return ret;
            }

            void *data = src.data(); // 获得浅拷贝指针

            // convert

            // Buffer *buffer = (*dst)->buffer;
            // if (buffer == nullptr)
            // {
            //     AbstractDevice *device = getDefaultHostDevice(); // 返回并创建cpudevice
            //     (*dst)->buffer = new Buffer()
            // }

            if (alloc)
            {
                //size_t size = 0;                  // GetByteSize(*blob);
                //AbstractDevice *device = nullptr; // getDefaultHostDevice();
                //(*dst)->buffer = new Buffer(device, size, data, true);
                // 深拷贝需要blob中的buffer额外创建一块内存
            }
            else
            {
                //
            }

            return RS_SUCCESS;
        }

    }
} // namespace rayshape::openvino