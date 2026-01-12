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
        RSMemoryInfo mem_info{mem_type, DataType::UINT8, (unsigned int)size};
        RSMemoryData mem_data{0, nullptr, nullptr};
        if ((Malloc(mem_info, mem_data)) == RS_SUCCESS) {
            Init(mem_info, mem_data.data_ptr_, mem_data.data_id_, false);
        }
    }

    Buffer::Buffer(const RSMemoryInfo &mem_info) {
        RSMemoryData mem_data{0, nullptr, nullptr};
        if ((Malloc(mem_info, mem_data)) == RS_SUCCESS) {
            Init(mem_info, mem_data.data_ptr_, mem_data.data_id_, false);
        }
    }

    Buffer::Buffer(void *data, size_t size, MemoryType mem_type) {
        RSMemoryInfo mem_info{mem_type, DataType::UINT8, (unsigned int)size};
        RSMemoryData mem_data{0, data, nullptr};
        Init(mem_info, mem_data.data_ptr_, mem_data.data_id_, true);
    }

    Buffer::Buffer(void *data, const RSMemoryInfo &mem_info) {
        RSMemoryData mem_data{0, data, nullptr};
        Init(mem_info, mem_data.data_ptr_, mem_data.data_id_, true);
    }

    Buffer::Buffer(unsigned int id, const RSMemoryInfo &mem_info) {
        RSMemoryData mem_data{id, nullptr, nullptr};
        if ((Malloc(mem_info, mem_data)) == RS_SUCCESS) {
            Init(mem_info, mem_data.data_ptr_, mem_data.data_id_, false);
        }
    }

    Buffer::~Buffer() {
        if (!is_external_) {
            if (mem_.mem_data_.data_ptr_) {
                AbstractDevice *device = (AbstractDevice *)mem_.mem_data_.context_;
                /*if (device == nullptr) {
                   device = GetDevice();
                }*/
                device->Free(mem_.mem_data_.data_ptr_);
                mem_.mem_data_.data_ptr_ = nullptr;
            }
        }
    }

    ErrorCode Buffer::Malloc(const RSMemoryInfo &mem_info, RSMemoryData &mem_data) {
        ErrorCode ret = RS_SUCCESS;

        DeviceType device_type = DeviceType::NONE;
        ret = ConvertMemoryTypeToDevice(mem_info.mem_type_, device_type);
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
        size_t byte_size = mem_info.size_ * GetBytesSize(mem_info.data_type_);
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
        mem_data.data_id_ = 0;              // default is zero
        mem_data.data_ptr_ = data_ptr;      // data pointer
        mem_data.context_ = (void *)device; // device_
        mem_.mem_data_.context_ = mem_data.context_;

        return ret;
    }

    void Buffer::Init(const RSMemoryInfo &mem_info, void *data, unsigned int data_id,
                      bool external) {
        mem_.mem_info_ = mem_info;

        mem_.mem_data_.data_ptr_ = data;
        mem_.mem_data_.data_id_ = data_id;
        is_external_ = external;
    }

    MemoryType Buffer::GetMemoryType() const {
        return mem_.mem_info_.mem_type_;
    }

    RSMemoryInfo Buffer::GetMemoryInfo() const {
        return mem_.mem_info_;
    }

    void *Buffer::GetDataPtr() const {
        if (mem_.mem_data_.data_ptr_ == nullptr) {
            return nullptr;
        } else {
            return mem_.mem_data_.data_ptr_;
        }
    }

    unsigned int Buffer::GetDataId() const {
        return mem_.mem_data_.data_id_;
    }

    size_t Buffer::GetDataSize() const {
        return mem_.mem_info_.size_;
    }

    bool Buffer::GetExternalFlag() const {
        return is_external_;
    }

    ErrorCode Buffer::DeepCopy(Buffer &dst) {
        ErrorCode ret = RS_SUCCESS;

        if (this == &dst) {
            RS_LOGI("src buffer obj equal dst buffer obj\n");
            return RS_SUCCESS;
        }
        // get src information and dst information.
        RSMemoryInfo src_info = this->GetMemoryInfo();
        RSMemoryInfo dst_info = dst.GetMemoryInfo();
        // allow different memory type.
        if (src_info.data_type_ != dst_info.data_type_ || src_info.size_ != dst_info.size_) {
            RS_LOGI("data_type or mem_size not equal\n");
            return RS_INVALID_PARAM;
        }

        // if (is_external_ == true || dst.is_external_ == true) {
        //     RS_LOGE("external alloc can not deep copy\n");
        //     return RS_NOT_IMPLEMENT;
        // }

        // transfer memmory_type to device_type
        DeviceType src_device_type = DeviceType::NONE;
        ret = ConvertMemoryTypeToDevice(src_info.mem_type_, src_device_type);
        if (ret != RS_SUCCESS) {
            RS_LOGE("ConvertMemoryTypeToDevice failed!\n");
            return ret;
        }
        AbstractDevice *src_device = GetDevice(src_device_type);

        DeviceType dst_device_type = DeviceType::NONE;
        ret = ConvertMemoryTypeToDevice(dst_info.mem_type_, dst_device_type);
        if (ret != RS_SUCCESS) {
            RS_LOGE("ConvertMemoryTypeToDevice failed!\n");
            return ret;
        }
        AbstractDevice *dst_device = GetDevice(dst_device_type);
        if (src_device == nullptr || dst_device == nullptr) {
            RS_LOGE("GetDevice src or dst failed!\n");
            return RS_DEVICE_NOT_SUPPORT;
        }

        if (src_info.mem_type_ == MemoryType::HOST && dst_info.mem_type_ == MemoryType::HOST) {
            return src_device->Copy(this, &dst, nullptr);
        } else if (src_info.mem_type_ == dst_info.mem_type_) {
            return src_device->Copy(this, &dst, nullptr);
        } else if (src_info.mem_type_ == MemoryType::HOST
                   && dst_info.mem_type_ != MemoryType::HOST) {
            return dst_device->CopyToDevice(this, &dst, nullptr);
        } else if (src_info.mem_type_ != MemoryType::HOST
                   && dst_info.mem_type_ == MemoryType::HOST) {
            return src_device->CopyFromDevice(this, &dst, nullptr); // down
        } else {
            RS_LOGE("not support memory type type{%d->%d} for buffer deepcopy operation.\n",
                    src_info.mem_type_, dst_info.mem_type_);
            return RS_NOT_IMPLEMENT;
        }
        return ret;
    }

    // 外部使用智能指针可以实现管理
    // 静态工厂方法用于动态创建并返回一个 Buffer 对象指针
    Buffer *Buffer::Alloc(size_t size, MemoryType mem_type) {
        if (size <= 0) {
            RS_LOGE("Invalid buffer size: %zu\n", size);
            return nullptr;
        }

        // creat tmp buffer
        Buffer *buffer = nullptr;
        buffer = new Buffer(size, mem_type);
        if (!buffer) {
            RS_LOGE("Failed to allocate Buffer object or allocate Buffer out of memmory.\n");
            return nullptr;
        }

        return buffer;
    }

    Buffer *Buffer::Alloc(const RSMemoryInfo &mem_info) {
        if (mem_info.size_ <= 0) {
            RS_LOGE("Invalid buffer size: %d\n", mem_info.size_);
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
        if (data == nullptr || mem_info.size_ <= 0) {
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
