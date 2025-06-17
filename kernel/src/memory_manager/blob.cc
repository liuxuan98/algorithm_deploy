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

        MemoryType mem_type = MEM_TYPE_NONE;
        auto ret = ConvertDeviceTypeToMemory(device_type, mem_type);
        if (ret != RS_SUCCESS) {
            RS_LOGE("ConvertDeviceTypeToMemory failed\n");
            delete blob;
            blob = nullptr;
            return nullptr;
        }
        size_t size = CalculateDims(blob->dims);
        RSMemoryInfo mem_info{ mem_type, data_type, size };
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

        if (dst_blob->buffer == nullptr || src_blob->buffer == nullptr) {
            RS_LOGE("src:%p or dst:%p Blob buffer is null.\n", dst_blob->buffer, src_blob->buffer);
            return RS_INVALID_PARAM;
        }

        // match size
        if (dst_blob->buffer->GetDataSize() != src_blob->buffer->GetDataSize()) {
            RS_LOGE("Destination buffer size (%zu) < source buffer size (%zu)\n",
                    dst_blob->buffer->GetDataSize(), src_blob->buffer->GetDataSize());
            return RS_INVALID_PARAM_VALUE;
        }

        // get src/dst device type.
        DeviceType src_device_type = src_blob->device_type;
        DeviceType dst_device_type = dst_blob->device_type;

        AbstractDevice *device = GetDevice(src_device_type);

        if (device == nullptr) {
            RS_LOGE("Failed to get device for type %d", src_device_type);
            return RS_DEVICE_INVALID;
        }

        // 如果是不同设备，需要中间拷贝到 CPU 再转到目标设备
        if (src_device_type != dst_device_type) {
            // 创建临时 CPU blob 缓冲区
            Blob *cpu_blob = BlobAlloc(DEVICE_TYPE_X86, src_blob->data_type, src_blob->data_format,
                                       "temp_cpu_blob", &src_blob->dims);
            if (!cpu_blob) {
                RS_LOGE("Failed to allocate temporary CPU blob\n");
                return RS_OUTOFMEMORY;
            }

            // 1. 设备 -> CPU
            if ((ret = device->CopyToDevice(cpu_blob->buffer, src_blob->buffer, nullptr))
                != RS_SUCCESS) {
                RS_LOGE("Failed to copy from source device to CPU: %d\n", ret);
                BlobFree(cpu_blob);
                return ret;
            }

            // 2. CPU -> 目标设备
            device = GetDevice(dst_device_type);
            if (!device) {
                RS_LOGE("Failed to get target device %d", dst_device_type);
                BlobFree(cpu_blob);
                return RS_DEVICE_INVALID;
            }

            if ((ret = device->CopyToDevice(dst_blob->buffer, cpu_blob->buffer, nullptr))
                != RS_SUCCESS) {
                RS_LOGE("Failed to copy from CPU to destination device: %d", ret);
                BlobFree(cpu_blob);
                return ret;
            }

            BlobFree(cpu_blob);
        } else {
            // 同设备类型，直接拷贝
            if ((ret = device->Copy(dst_blob->buffer, src_blob->buffer, nullptr)) != RS_SUCCESS) {
                RS_LOGE("Failed to copy within same device: %d.\n", ret);
                return ret;
            }
        }

        return ret;
    }

    ErrorCode BlobFree(Blob *blob) { // 线程不安全，智能指针封装

        if (!blob) {
            RS_LOGE("Blob pointer is null");
            return RS_INVALID_PARAM;
        }

        // 如果 buffer 不为 nullptr，则释放它
        if (blob->buffer != nullptr) {
            delete blob->buffer;
            blob->buffer = nullptr;
        }

        // 释放 Blob 结构体本身
        free(blob);

        return RS_SUCCESS;
    }

} // namespace rayshape
