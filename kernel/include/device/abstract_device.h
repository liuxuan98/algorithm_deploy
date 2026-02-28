/**
 * @file abstract_device.h
 * @brief 设备内存管理的抽象模块
 * @copyright .
 *
 *
 * @author Liuxuan
 * @email liuxuan@rayshape.com
 * @date 2025-05-16
 * @version 1.0.0
 */

#ifndef ABSTRACT_DEVICE_H
#define ABSTRACT_DEVICE_H

#include "base/common.h"
#include "base/error.h"
#include "base/macros.h"
#include "base/logger.h"
#include "memory_manager/buffer.h"
#include "utils/type_utils.h"

using namespace rayshape::utils;

namespace rayshape
{
    namespace device
    {
        /**
         * @brief NonCopyable is abstract device base class.
         *
         *
         */
        class RS_PUBLIC NonCopyable {
        public:
            NonCopyable() = default;

            NonCopyable(const NonCopyable &) = delete;
            NonCopyable &operator=(const NonCopyable &) = delete;

            NonCopyable(NonCopyable &&) = delete;
            NonCopyable &operator=(NonCopyable &&) = delete;
        };
        /**
         * @brief Abstract device class.
         * @details 依据不同设备类型实现不同的内存管理(分配,释放,拷贝,etc..)
         * 统一设备管理的接口，提供内存分配,释放,拷贝,拷贝到设备,拷贝从设备等功能
         */
        class RS_PUBLIC AbstractDevice: public NonCopyable {
        public:
            // @brief constructor
            explicit AbstractDevice(DeviceType);

            // @brief virtual destructor
            virtual ~AbstractDevice();

        public:
            /**
             * @brief allocate memory by size in specific device
             * @param[in] size memory size(byte)
             * @param[out] ptr memory pointer
             * @return ErrorCode RS_SUCCESS if copy success, otherwise error code
             */
            virtual ErrorCode Allocate(size_t size, void **ptr) = 0;

            /**
             * @brief free memory in specific device
             * @param[in] ptr memory pointer
             * @return ErrorCode RS_SUCCESS if copy success, otherwise error code
             */
            virtual ErrorCode Free(void *ptr) = 0;

            /**
             * @brief memory copy in same device type
             * @param[in] src src device memory pointer
             * @param[in] dst dst device memory pointer
             * @param[in] size need copy memory size(byte)
             * @return ErrorCode RS_SUCCESS if copy success, otherwise error code
             */
            virtual ErrorCode Copy(const void *src, void *dst, size_t size,
                                   void *command_queue) = 0;

            /**
             * @brief memory copy to device for host to device
             * @param[in] src src host device memory pointer
             * @param[in] dst dst device memory pointer
             * @param[in] size need copy memory size(byte)
             * @return ErrorCode RS_SUCCESS if copy success, otherwise error code
             */
            virtual ErrorCode CopyToDevice(const void *src, void *dst, size_t size,
                                           void *command_queue) = 0;
            /**
             * @brief memory copy from device for device to host
             * @param[in] src src device memory pointer
             * @param[in] dst dst host device memory pointer
             * @param[in] size need copy memory size(byte)
             * @return ErrorCode RS_SUCCESS if copy success, otherwise error code
             */
            virtual ErrorCode CopyFromDevice(const void *src, void *dst, size_t size,
                                             void *command_queue) = 0;

            /**
             * @brief memory copy by buffer in same device type
             * @param[in] src src Buffer pointer
             * @param[in] dst dst Buffer pointer
             * @return ErrorCode RS_SUCCESS if copy success, otherwise error code
             */
            virtual ErrorCode Copy(const Buffer *src, Buffer *dst, void *command_queue) = 0;

            /**
             * @brief memory copy by buffer for host to device
             * @param[in] src src host Buffer pointer
             * @param[in] dst dst dvice Buffer pointer
             * @return ErrorCode RS_SUCCESS if copy success, otherwise error code
             */
            virtual ErrorCode CopyToDevice(const Buffer *src, Buffer *dst, void *command_queue) = 0;

            /**
             * @brief memory copy by buffer for device to host
             * @param[in] src src dvice Buffer pointer
             * @param[in] dst dst host Buffer pointer
             * @return ErrorCode RS_SUCCESS if copy success, otherwise error code
             */
            virtual ErrorCode CopyFromDevice(const Buffer *src, Buffer *dst,
                                             void *command_queue) = 0;

            /**
             * @brief Get Device Type
             * @return DeviceType CPU,CUDA,OPENCL,etc...
             */
            DeviceType GetDeviceType();

        protected:
            // protected member
            DeviceType device_type_;
        };

        // @brief GetGlobalDeviceMap device type map
        std::map<DeviceType, std::shared_ptr<AbstractDevice>> &GetGlobalDeviceMap();

        // @brief Get Device Object Pointer By DeviceType
        RS_PUBLIC AbstractDevice *GetDevice(DeviceType type);

        // @brief TypeDeviceRegister construct register device
        template <typename T> class TypeDeviceRegister {
        public:
            explicit TypeDeviceRegister(DeviceType type) {
                auto &device_map = GetGlobalDeviceMap();
                if (device_map.find(type) == device_map.end()) {
                    device_map[type] = std::shared_ptr<T>(new T(type));
                }
            }
        };

        bool IsHostDeviceType(DeviceType device_type);

    } // namespace device
} // namespace rayshape

#endif // ABSTRACT_DEVICE_H
