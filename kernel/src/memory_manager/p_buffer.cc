// #include "memory_manager/p_buffer.h"
// #include "device/abstract_device.h"

// using namespace rayshape::device;

// namespace rayshape
// {
//     Buffer::Buffer() : size_(0), data_(nullptr), mem_type_(DEVICE_TYPE_NONE), external_(false) {}

//     Buffer::~Buffer() {
//         if (external_) {
//             data_ = nullptr;
//             data_alloc_ = nullptr;
//         } else {
//             // todo
//         }
//     }
//     // 内部分配
//     // ErrorCode Buffer::CreateBuffer(size_t byte_size, DeviceType mem_type, bool external_alloc) {
//     //    AbstractDevice *device = GetDevice(device_type);
//     //    if (device == nullptr) {
//     //        RS_LOGE("get abstract device failed, not found this device type:%d.\n", device_type);
//     //        return RS_DEVICE_INVALID;
//     //    }
//     //    mem_type_ = device_type;

//     //    if (byte_size <= 0) {
//     //        RS_LOGE("Invalid buffer size: %zu.\n", byte_size);
//     //        return RS_INVALID_PARAM_VALUE;
//     //    }
//     //    size_ = byte_size;

//     //    void *data_alloc = nullptr;
//     //    ErrorCode rs_err = device->Allocate(byte_size, &data_alloc);
//     //    if (rs_err == RS_SUCCESS) {
//     //        data_alloc_ = std::shared_ptr<void>(data_alloc, [=](void *p) {
//     //            auto device = GetDevice(device_type);
//     //            if (device) {
//     //                device->Free(p);
//     //            }
//     //        });
//     //        data_ = data_alloc_.get(); // 放在析构也可
//     //    } else {
//     //        RS_LOGE("device allocate failed with error code: %d.\n", err);
//     //        data_ = nullptr;
//     //        data_alloc_ = nullptr;
//     //        return rs_err;
//     //    }

//     //    external_ = false;
//     //    return rs_err;
//     //}

//     /*   ErrorCode Buffer::CreateBuffer(size_t byte_size, DeviceType mem_type, bool external_alloc){


//        }*/
//     Buffer::Buffer(size_t byte_size, DeviceType device_type, bool external_alloc) {
//         AbstractDevice *device = GetDevice(device_type);
//         // ASSERT(device != NULL);
//         if (device == nullptr) {
//             RS_LOGE("get abstract device failed, not found this device type:%d.\n", device_type);
//             return;
//         }
//         mem_type_ = device_type;

//         size_ = byte_size;
//         if (byte_size <= 0) {
//             RS_LOGE("Invalid buffer size: %zu", byte_size);
//             return;
//         }

//         void *data_alloc = nullptr;
//         ErrorCode err = device->Allocate(byte_size, &data_alloc);
//         if (err == RS_SUCCESS) {
//             data_alloc_ = std::shared_ptr<void>(data_alloc, [=](void *p) {
//                 auto device = GetDevice(device_type);
//                 if (device) {
//                     device->Free(p);
//                 }
//             });
//             data_ = data_alloc_.get(); // 放在析构也可
//         } else {
//             RS_LOGE("device allocate failed with error code: %d", err);
//             data_ = nullptr;
//             data_alloc_ = nullptr;
//         }

//         external_ = false;
//     }
//     // 外部分配的内存外部管理

//     Buffer::Buffer(size_t byte_size, DeviceType mem_type, bool external_alloc, void *data) {
//         if (data == nullptr) {
//             RS_LOGE("external data:%p is null", data);
//         }
//         data_alloc_ = nullptr;

//         mem_type_ = mem_type;

//         external_ = external_alloc;

//         data_ = data;

//         size_ = byte_size;
//     }

//     void *Buffer::GetSrcData() const {
//         return data_;
//     }

//     size_t Buffer::GetSize() const {
//         return size_;
//     }

//     bool Buffer::GetAllocFlag() const {
//         return external_;
//     }
// } // namespace rayshape