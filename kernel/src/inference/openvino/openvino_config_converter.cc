#include "inference/openvino/openvino_config_converter.h"

namespace rayshape
{
    namespace openvino
    {

        DataType OpenVINOConfigConverter::ConvertToDataType(const ov::element::Type &precision)
        {
            if (ov::element::f32 == precision)
            {
                return DataType::DATA_TYPE_FLOAT;
            }
            else if (ov::element::f16 == precision)
            {
                return DataType::DATA_TYPE_HALF;
            }
            else if (ov::element::u16 == precision)
            {
            }
            else if (ov::element::u8 == precision)
            {
                // todo ....
            }
            else
            {
                // RS_LOGE("ERROR:OpenVINOConfigConverter ConvertToDataType precision\n", precision);
                return DATA_TYPE_AUTO;
            }
            return DATA_TYPE_AUTO;
        }

        ErrorCode OpenVINOConfigConverter::ConvertFromDataType(const ov::element::Type &precision, const DataType dataType)
        {

            return RS_SUCCESS;
        }

        ErrorCode OpenVINOConfigConverter::ConvertFromDims(ov::Shape &dst, const Dims &src)
        {

            for (int i = 0; i < src.size; i++)
            {
                dst.emplace_back(static_cast<size_t>(src.value[i]));
            }

            return RS_SUCCESS;
        }

        ErrorCode OpenVINOConfigConverter::ConvertToDims(Dims &dst, const ov::Shape &src)
        {
            if (dst.size != src.size())
            {
                return RS_INVALID_PARAM;
            }
            for (int i = 0; i < src.size(); i++)
            {
                dst.value[i] = src[i];
            }
        }

        ErrorCode OpenVINOConfigConverter::ConvertToDevice(DeviceType &dst, const std::string &src)
        {
            if (src == "CPU")
            {
                dst = DEVICE_TYPE_X86;
            }
            else if (src == "GPU")
            {
                dst = DEVICE_TYPE_OPENCL;
            }
            else
            {
                dst = DEVICE_TYPE_X86;
            }

            return RS_SUCCESS;
        }
        ErrorCode OpenVINOConfigConverter::ConvertFromDevice(std::string &dst, const DeviceType &src)
        {
            if (DEVICE_TYPE_X86 == src)
            {
                dst = "CPU";
            }
            else if (DEVICE_TYPE_OPENCL == src)
            {
                dst = "GPU";
            }
            else if (DEVICE_TYPE_INTERL_NPU == src)
            {
                dst = "AUTO:NPU,CPU";
            }
            else
            {
                dst = "CPU";
            }
            return RS_SUCCESS;
        }

        DataFormat OpenVINOConfigConverter::ConvertToDataFormat(const std::string &src)
        {
            if ("NCHW" == src)
            {
                return DATA_FORMAT_NCHW;
            }
            else if ("NHWC" == src)
            {
                return DATA_FORMAT_NHWC;
            }
            else if ("NC" == src)
            {
                return DATA_FORMAT_NC;
            }
            else if ("NCDHW" == src)
            {
                return DATA_FORMAT_NCDHW;
            }
            else
            {
                return DATA_FORMAT_AUTO;
            } // 暂时先列举这些数据格式
        }

        ErrorCode OpenVINOConfigConverter::ConvertFromDataFormat(ov::Layout &dst, const DataFormat src)
        {
            switch (src)
            {
            case DATA_FORMAT_NCHW:
                dst = ov::Layout("NCHW");
                break;
            case DATA_FORMAT_NHWC:
                dst = ov::Layout("NHWC");
                break;
            case DATA_FORMAT_NC:
                dst = ov::Layout("NC");
                break;
            case DATA_FORMAT_NCDHW:
                dst = ov::Layout("NCDHW");
                break;
            }

            return RS_SUCCESS;
        }

        // ErrorCode

    }
} // namespace rayshape::openvino
