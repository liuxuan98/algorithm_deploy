#include "memory_manager/buffer.h"
#include "utils/device_convert_utils.h"
#include "utils/type_utils.h"
#include "device/abstract_device.h"

using namespace rayshape::utils;
using namespace rayshape::device;

namespace rayshape
{
    Buffer::Buffer() {}

    Buffer::Buffer(size_t size, MemoryType mem_type) {
        RSMemoryInfo mem_info{ mem_type, DATA_TYPE_UINT8, (unsigned int)size };
        RSMemoryData mem_data{ 0, nullptr, nullptr };
        if ((Malloc(mem_info, mem_data)) == RS_SUCCESS) {
            Init(mem_info, mem_data.data_ptr, mem_data.data_id, false);
        }
    }

    Buffer::Buffer(const RSMemoryInfo &mem_info) {
        RSMemoryData mem_data{ 0, nullptr, nullptr };
        if ((Malloc(mem_info, mem_data)) == RS_SUCCESS) {
            Init(mem_info, mem_data.data_ptr, mem_data.data_id, false);
        }
    }

    Buffer::Buffer(void *data, size_t size, MemoryType mem_type) {
        RSMemoryInfo mem_info{ mem_type, DATA_TYPE_UINT8, (unsigned int)size };
        RSMemoryData mem_data{ 0, data, nullptr };
        Init(mem_info, mem_data.data_ptr, mem_data.data_id, true);
    }

    Buffer::Buffer(void *data, const RSMemoryInfo &mem_info) {
        RSMemoryData mem_data{ 0, data, nullptr };
        Init(mem_info, mem_data.data_ptr, mem_data.data_id, true);
    }

    Buffer::Buffer(unsigned int id, const RSMemoryInfo &mem_info) {
        RSMemoryData mem_data{ id, nullptr, nullptr };
        if ((Malloc(mem_info, mem_data)) == RS_SUCCESS) {
            Init(mem_info, mem_data.data_ptr, mem_data.data_id, false);
        }
    }

    Buffer::~Buffer() {
        if (!is_external_) {
            if (mem_.mem_data.data_ptr) {
                AbstractDevice *device = (AbstractDevice *)mem_.mem_data.context;
                /*if (device == nullptr) {
                   device = GetDevice();
                }*/
                device->Free(mem_.mem_data.data_ptr);
                mem_.mem_data.data_ptr = nullptr;
            }
        }
    }

    ErrorCode Buffer::Malloc(const RSMemoryInfo &mem_info, RSMemoryData &mem_data) {
        ErrorCode ret = RS_SUCCESS;

        DeviceType device_type = DEVICE_TYPE_NONE;
        ret = ConvertMemoryTypeToDevice(mem_info.men_type, device_type);
        if (ret != RS_SUCCESS) {
            RS_LOGE("ConvertMemoryTypeToDevice failed!\n");
            return ret;
        }
        AbstractDevice *device = GetDevice(device_type);
        if (device == nullptr) {
            RS_LOGE("get abstract device failed, not found this device type:%d.\n", device_type);
            return RS_DEVICE_INVALID;
        }

        // device mem alloc
        // byte size calculate
        size_t byte_size = mem_info.size * GetBytesSize(mem_info.data_type);
        if (byte_size <= 0) {
            RS_LOGE("can not alloc:%zu size memory.\n", byte_size);
            return RS_INVALID_PARAM_VALUE;
        }
        void *data_ptr = nullptr;
        ret = device->Allocate(byte_size, &data_ptr);
        if (ret != RS_SUCCESS) {
            RS_LOGE("device:%d Memory allocation failed with error code: %d \n", device_type, ret);
            return ret;
        }
        if (data_ptr == nullptr) {
            RS_LOGE("alloc size error or alloc out of memory.\n");
            return RS_OUTOFMEMORY;
        }

        // full in data and return.
        mem_data.data_id = 0;              // default is zero
        mem_data.data_ptr = data_ptr;      // data pointer
        mem_data.context = (void *)device; // device_

        return ret;
    }

    void Buffer::Init(const RSMemoryInfo &mem_info, void *data, unsigned int data_id,
                      bool external) {
        mem_.mem_info = mem_info;

        mem_.mem_data.data_ptr = data;
        mem_.mem_data.data_id = data_id;
        is_external_ = external;
    }

    MemoryType Buffer::GetMemoryType() const {
        return mem_.mem_info.men_type;
    }

    RSMemoryInfo Buffer::GetMemoryInfo() const {
        return mem_.mem_info;
    }

    void *Buffer::GetDataPtr() const {
        if (mem_.mem_data.data_ptr == nullptr) {
            return nullptr;
        } else {
            return mem_.mem_data.data_ptr;
        }
    }

    unsigned int Buffer::GetDataId() const {
        return mem_.mem_data.data_id;
    }

    size_t Buffer::GetDataSize() const {
        return mem_.mem_info.size;
    }

    bool Buffer::GetExternalFlag() const {
        return is_external_;
    }

    // 深拷贝src to dst.only support same device copy
    ErrorCode Buffer::DeepCopy(Buffer &dst) {
        ErrorCode ret = RS_SUCCESS;

        // check null pointer
        if (&dst == nullptr) {
            RS_LOGE("Invalid destination buffer object is null.\n");
            return RS_INVALID_PARAM;
        }

        // check
        if (mem_.mem_info.men_type != dst.mem_.mem_info.men_type
            || mem_.mem_info.data_type != dst.mem_.mem_info.data_type
            || mem_.mem_info.size != dst.mem_.mem_info.size) {
            return RS_INVALID_PARAM;
        }

        if (is_external_ == true || dst.is_external_ == true) {
            RS_LOGE("external alloc can not deep copy");
            return RS_NOT_IMPLEMENT;
        }

        if (mem_.mem_data.data_ptr == nullptr || dst.mem_.mem_data.data_ptr == nullptr) {
            RS_LOGE("src or dst data ptr is null\n");
            return RS_INVALID_PARAM;
        }

        //  如果是同种设备类型，直接拷贝数据
        size_t copy_size = dst.GetDataSize() * GetBytesSize(dst.mem_.mem_info.data_type);
        if (mem_.mem_info.men_type == dst.GetMemoryType()) {
            AbstractDevice *device = (AbstractDevice *)mem_.mem_data.context;
            ret = device->Copy(mem_.mem_data.data_ptr, dst.mem_.mem_data.data_ptr,
                               dst.GetDataSize(), nullptr);
            if (ret != RS_SUCCESS) {
                RS_LOGE(
                    "Copy data from source device to destination device failed with error code: %d \n",
                    ret);
                return ret;
            }
        } else {
            // todo
        }

        return RS_SUCCESS;
    }

    // 外部使用智能指针可以实现管理
    // 静态工厂方法用于动态创建并返回一个 Buffer 对象指针
    Buffer *Buffer::Alloc(size_t size, MemoryType mem_type) {
        if (size <= 0) {
            RS_LOGE("Invalid buffer size: %zu\n", size);
            return nullptr;
        }

        // 创建临时对象
        Buffer *buffer = nullptr;
        buffer = new Buffer(size, mem_type);
        if (!buffer) {
            RS_LOGE("Failed to allocate Buffer object or allocate Buffer out of memmory.\n");
            return nullptr;
        }

        return buffer;
    }

    Buffer *Buffer::Alloc(const RSMemoryInfo &mem_info) {
        if (mem_info.size <= 0) {
            RS_LOGE("Invalid buffer size: %d\n", mem_info.size);
            return nullptr;
        }

        Buffer *buffer = new Buffer(mem_info);
        if (!buffer) {
            RS_LOGE("Failed to allocate Buffer object\n");
            return nullptr;
        }

        return buffer;
    }

    Buffer *Buffer::Create(void *data, size_t size, MemoryType mem_type) {
        if (data == nullptr || size <= 0) {
            RS_LOGE("Invalid data or size\n");
            return nullptr;
        }
        Buffer *buffer = new Buffer(data, size, mem_type);
        if (!buffer) {
            RS_LOGE("Failed to allocate Buffer object\n");
            return nullptr;
        }
        return buffer;
    }

    Buffer *Buffer::Create(void *data, const RSMemoryInfo &mem_info) {
        if (data == nullptr || mem_info.size <= 0) {
            RS_LOGE("Invalid data or mem size <=0\n");
            return nullptr;
        }

        Buffer *buffer = new Buffer(data, mem_info);
        if (!buffer) {
            RS_LOGE("Failed to allocate Buffer object\n");
            return nullptr;
        }
        return buffer;
    }

    Buffer *Buffer::Create(unsigned int id, const RSMemoryInfo &mem_info) {
        Buffer *buffer = new Buffer(id, mem_info);
        if (!buffer) {
            RS_LOGE("Failed to allocate Buffer object\n");
            return nullptr;
        }
        return buffer;
    }

    void *RSBufferDataGet(Buffer *buffer) {
        if (buffer == nullptr) {
            return nullptr;
        }
        return buffer->GetDataPtr();
    }

} // namespace rayshape
