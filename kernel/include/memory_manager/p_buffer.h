// /**
//  * @file buffer.h
//  * @brief buffer内存管理模块
//  * @copyright (c) 2025 Shenzhen RayShape Medical Technology Co., Ltd.
//  *
//  *
//  * @author Liuxuan
//  * @email liuxuan@rayshape.com
//  * @date 2025-05-16
//  * @version 1.0.0
//  */

// #ifndef _BUFFER_H_
// #define _BUFFER_H_

// #include <memory>
// #include <string>

// #include "base/common.h"
// #include "base/error.h"

// namespace rayshape
// {
//     class RS_PUBLIC Buffer {
//     public:
//         Buffer();
//         // 接口变动
//         ~Buffer();

//         ErrorCode CreateBuffer(size_t byte_size, DeviceType mem_type, bool external_alloc = false);

//         ErrorCode CreateBuffer(size_t byte_size, DeviceType mem_type, void *data,
//                                bool external_alloc = true);
//         /**
//          * @brief explicit constructor with inner alloc memory
//          * @param[in] byte_size memory size(byte)
//          * @param[in] mem_type memory type depend on device
//          * @param[in] external_alloc false mean inner alloc,true mean external alloc
//          * @return Buffer
//          */
//         explicit Buffer(size_t byte_size, DeviceType mem_type, bool external_alloc);
//         /**
//          * @brief explicit constructor with external alloc memory
//          * @param[in] byte_size memory size(byte)
//          * @param[in] mem_type memory type depend on device
//          * @param[in] external_alloc false mean inner alloc,true mean external alloc
//          * @param[in] data external alloc memory pointer
//          * @return Buffer
//          */
//         explicit Buffer(size_t byte_size, DeviceType mem_type, bool external_alloc, void *data);

//         /**
//          * @brief get buffer memory data pointer
//          * @return void * memory pointer
//          */

//         void *GetSrcData() const; // 考虑将此接口封装一层
//         /**
//          * @brief get memory data size
//          * @return size_t
//          */
//         size_t GetSize() const;
//         /**
//          * @brief get alloc is inner or external flag
//          * @return bool
//          */
//         bool GetAllocFlag() const;

//     private:
//         std::shared_ptr<void> data_alloc_ = nullptr;

//         void *data_ = nullptr;
//         size_t size_ = 0;

//         DeviceType mem_type_ = DEVICE_TYPE_NONE;

//         bool external_ = false;
//     };

// } // namespace rayshape

// #endif