#include "memory_manager/blob.h"
#include "device/abstract_device.h"
#include "utils/memory_size_info.h"
#include "utils/device_convert_utils.h"

using namespace rayshape::device;
using namespace rayshape::utils;

namespace rayshape
{
    Blob *BlobAlloc(DeviceType device_type, DataType data_type, DataFormat data_format,
                    const char *name, const Dims *dims) {
        if (name == nullptr || dims == nullptr) {
            RS_LOGE("name:%s or dims:%p is null.\n", name, dims);
            return nullptr;
        }

        Blob *blob = (Blob *)malloc(sizeof(Blob));
        memset(blob, 0, sizeof(Blob));

        blob->device_type = device_type;
        blob->data_type = data_type;
        blob->data_format = data_format;

        if (strlen(name) < MAX_BLOB_NAME) {
            strncpy(blob->name, name, strlen(name));
        } else {
            RS_LOGW("name length:%d is bigger than MAX_BLOB_NAME:%d\n", (int)strlen(name),
                    MAX_BLOB_NAME);
            strncpy(blob->name, name, MAX_BLOB_NAME);
        }
        blob->dims = *dims;

        MemoryType mem_type = MemoryType::NONE;
        auto ret = ConvertDeviceTypeToMemory(device_type, mem_type);
        if (ret != RS_SUCCESS) {
            RS_LOGE("ConvertDeviceTypeToMemory failed\n");
            delete blob;
            blob = nullptr;
            return nullptr;
        }
        size_t size = CalculateDims(blob->dims);
        RSMemoryInfo mem_info{mem_type, data_type, static_cast<unsigned int>(size)};
        Buffer *buffer = nullptr;
        buffer = Buffer::Alloc(mem_info);
        if (buffer == nullptr) {
            RS_LOGE("new Buffer failed\n");
            delete blob;
            blob = nullptr;
            return nullptr;
        }
        blob->buffer = buffer;

        return blob;
    }

    size_t BlobSizeGet(const Blob *blob) {
        if (blob == nullptr) {
            RS_LOGE("blob is null.\n");
            return 0;
        }
        Buffer *buf = (Buffer *)blob->buffer;
        size_t size = buf->GetDataSize();
        return size;
    }

    ErrorCode BlobCopy(const Blob *src_blob, Blob *dst_blob) {
        ErrorCode ret = RS_SUCCESS;

        if (dst_blob == nullptr || src_blob == nullptr) {
            RS_LOGE("Invalid blob pointer: dst=%p, src=%p.\n", dst_blob, src_blob);
            return RS_INVALID_PARAM;
        }

        if ((dst_blob->data_type != src_blob->data_type)
            || (dst_blob->data_format != src_blob->data_format)) {
            RS_LOGE("src blob data_type:%d,data_format:%d;dst blob data_type:%d,data_format:%d.\n",
                    src_blob->data_type, src_blob->data_format, dst_blob->data_type,
                    dst_blob->data_format);
            return RS_INVALID_PARAM;
        }

        Buffer *src_buf = src_blob->buffer;
        Buffer *dst_buf = dst_blob->buffer;

        if (dst_buf == nullptr || src_buf == nullptr) {
            RS_LOGE("src:%p or dst:%p Blob buffer is null.\n", src_buf, dst_buf);
            return RS_INVALID_PARAM;
        }

        if (dst_buf == src_buf) {
            RS_LOGE("dst_blob and src_blob are the same pointer.\n");
            return RS_INVALID_PARAM;
        }

        // match size or check dims.
        if (dst_buf->GetDataSize() != src_buf->GetDataSize()) {
            RS_LOGE("Destination buffer size (%zu) < source buffer size (%zu)\n",
                    dst_buf->GetDataSize(), src_buf->GetDataSize());
            return RS_INVALID_PARAM_VALUE;
        }

        // get src/dst device type.
        DeviceType src_device_type = src_blob->device_type;
        DeviceType dst_device_type = dst_blob->device_type;

        AbstractDevice *src_device = GetDevice(src_device_type);
        AbstractDevice *dst_device = GetDevice(dst_device_type);
        if (src_device == nullptr || dst_device == nullptr) {
            RS_LOGE("src blob device:%d or dst blob device:%d not get.\n", src_device_type,
                    dst_device_type);
            return RS_DEVICE_INVALID;
        }

        if (src_device_type == dst_device_type) {
            ret = src_device->Copy(src_buf, dst_buf, nullptr);
            if (ret != RS_SUCCESS) {
                RS_LOGE("Copy from src blob to dst blob failed.\n");
                return ret;
            }
        } else if (IsHostDeviceType(src_device_type) && IsHostDeviceType(dst_device_type)) {
            ret = src_device->Copy(src_buf, dst_buf, nullptr);
            if (ret != RS_SUCCESS) {
                RS_LOGE("Copy from src blob to dst blob failed.\n");
                return ret;
            }
        } else if (IsHostDeviceType(src_device_type) && !IsHostDeviceType(dst_device_type)) {
            ret = dst_device->CopyToDevice(src_buf, dst_buf, nullptr);
            if (ret != RS_SUCCESS) {
                RS_LOGE("CopyToDevice from src blob to dst blob failed.\n");
                return ret;
            }
        } else if (!IsHostDeviceType(src_device_type) && IsHostDeviceType(dst_device_type)) {
            ret = src_device->CopyFromDevice(src_buf, dst_buf, nullptr);
            if (ret != RS_SUCCESS) {
                RS_LOGE("CopyFromDevice from src blob to dst blob failed.\n");
                return ret;
            }
        } else {
            // different type Heterogeneous Computing device.
            DeviceType host_divice_type = DeviceType::CPU;
            AbstractDevice *host_device = GetDevice(host_divice_type);
            if (host_device == nullptr) {
                RS_LOGE("host device is invalid.\n");
                return RS_DEVICE_INVALID;
            }
            // Create temporary Host blob as intermediate
            Blob *cpu_blob = BlobAlloc(host_divice_type, src_blob->data_type, src_blob->data_format,
                                       "temp_cpu_blob", &src_blob->dims);
            if (!cpu_blob) {
                RS_LOGE("Failed to allocate temporary CPU blob\n");
                return RS_OUTOFMEMORY;
            }

            // 1.src device to cpu
            if ((ret = src_device->CopyFromDevice(src_buf, cpu_blob->buffer, nullptr))
                != RS_SUCCESS) {
                RS_LOGE("Failed to copy from source device to CPU: %d\n", ret);
                BlobFree(cpu_blob);
                return ret;
            }

            // 2.cpu to dst device
            if ((ret = dst_device->CopyToDevice(cpu_blob->buffer, dst_buf, nullptr))
                != RS_SUCCESS) {
                RS_LOGE("Failed to copy CPU to dst device: %d\n", ret);
                BlobFree(cpu_blob);
                return ret;
            }

            BlobFree(cpu_blob);
        }

        return ret;
    }

    ErrorCode BlobFree(Blob *blob) { // no safe for threads. shared_ptr holder
        if (blob == nullptr) {
            RS_LOGE("Blob pointer is null\n");
            return RS_INVALID_PARAM;
        }
        // if buffer is not nullptr, otherwise delete it.
        if (blob->buffer != nullptr) {
            delete blob->buffer;
            blob->buffer = nullptr;
        } else {
            RS_LOGE("Blob buffer pointer is null\n");
            return RS_INVALID_PARAM;
        }
        // delete Blob struct
        free(blob);
        return RS_SUCCESS;
    }

} // namespace rayshape
