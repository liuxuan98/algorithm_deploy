#include "memory_manager/blob.h"
#include "device/abstract_device.h"

using namespace rayshape::device;

namespace rayshape
{
    Blob *BlobAlloc(DeviceType device_type, DataType data_type, DataFormat data_format,
                    const char *name, const Dims *dims)
    {
        // 分配结构体
        Blob *blob = (Blob *)malloc(sizeof(Blob));

        AbstractDevice *device = GetDevice(device_type);

        blob->device_type = device_type;
        blob->data_type = data_type;
        blob->data_format = data_format;
        //blob->name = std::string(*name).c_str();  //
        blob->dims = *dims;
        // 计算工具计算size
        size_t size = 50;
        // void *ptr = device->allocate(size);
        blob->buffer = new Buffer(size, device_type,true); // 内部调用分配


        return blob;
    }

    void *BlobBufferGet(Blob *blob)
    {
        return blob->buffer;
    }

    size_t BlobSizeGet(Blob *blob)
    {
        if (blob == nullptr)
        {
            return 0;
        }
        return 0;
    }

    ErrorCode BlobCopy(Blob *dst_blob, Blob *src_blob)
    {
        return RS_SUCCESS;
    }

    ErrorCode BlobFree(Blob *blob)
    {
        return RS_SUCCESS;
    }

}
