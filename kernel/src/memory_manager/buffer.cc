
#include "memory_manager/buffer.h"
#include "device/abstract_device.h"

using namespace rayshape::device;

namespace rayshape
{
    Buffer::Buffer() : size_(0), data_(nullptr), mem_type_(DEVICE_TYPE_NONE), external_(false)
    {
    }

    Buffer::~Buffer()
    {
        if (external_)
        {
            data_ = nullptr;
            data_alloc_ = nullptr;
        }
        else
        {
            // todo
        }
    }
    // 内部分配
    Buffer::Buffer(size_t byte_size, DeviceType device_type, bool external_alloc)
    {
        mem_type_ = device_type;
        auto device = GetDevice(device_type);
        if (device == nullptr)
        {
            // 日志
        }
        size_ = byte_size;
        if (byte_size <= 0)
        {
            // 日志
        }

        void *data_alloc = nullptr;

        ErrorCode err = device->Allocate(byte_size, &data_alloc);
        if (err == RS_SUCCESS)
        {

            data_alloc_ = std::shared_ptr<void>(data_alloc, [=](void *p)
                                                {
            auto device = GetDevice(device_type);
            if (device) {
                device->Free(p);
            } });
            data_ = data_alloc_.get(); // 放在析构也可
        }
        else
        {
            data_ = nullptr;
            data_alloc_ = nullptr;
        }

        external_ = false;
    }
    // 外部分配的内存外部管理
    Buffer::Buffer(size_t byte_size, DeviceType mem_type, bool external_alloc, void *data)
    {
        if (data == nullptr)
        {
            // 日志
        }
        data_alloc_ = nullptr;

        mem_type_ = mem_type;

        external_ = external_alloc;

        data_ = data;

        size_ = byte_size;
    }

    void *Buffer::GetSrcData()
    {
        return data_;
    }

    bool Buffer::GetAllocFlag() const
    {
        return external_;
    }

}