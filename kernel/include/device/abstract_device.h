#ifndef _ABSTRACT_DEVICE_H_
#define _ABSTRACT_DEVICE_H_

#include "base/common.h"
#include "base/error.h"
#include "base/macros.h"
#include "memory_manager/buffer.h"

namespace rayshape
{
    namespace device
    {
        class RS_PUBLIC NonCopyable
        {
        public:
            NonCopyable() = default;

            NonCopyable(const NonCopyable &) = delete;
            NonCopyable &operator=(const NonCopyable &) = delete;

            NonCopyable(NonCopyable &&) = delete;
            NonCopyable &operator=(NonCopyable &&) = delete;
        };
        // 抽象设备可以给blob和buffer分配内存(在不同设备上)
        class RS_PUBLIC AbstractDevice : public NonCopyable
        {
        public:
            // @brief constructor
            explicit AbstractDevice(DeviceType);

            // @brief virtual destructor
            virtual ~AbstractDevice();

        public:
            // @brief allocate memory by size in specific device
            // @return allocated memory pointer
            virtual ErrorCode Allocate(size_t size, void **ptr) = 0;

            // @brief deallocate memory by pointer
            // @return return void
            virtual ErrorCode Free(void *ptr) = 0;

            // @brief copy memory from host to host,or device to device
            // @return _OK if copy success, otherwise error code.
            virtual ErrorCode Copy(void *src, void *dst, size_t size,
                                   void *command_queue = nullptr) = 0;

            // @brief Transfer memory from Host to Device
            // @return _OK if copy success, otherwise error code.
            virtual ErrorCode CopyToDevice(void *src, void *dst, size_t size) = 0;
            // @brief Transfer memory from Device to Host
            // @return _OK if copy success, otherwise error code.
            virtual ErrorCode CopyFromDevice(void *src, void *dst, size_t size) = 0;

            // @brief copy memory from host to host,or device to device ,by Buffer
            // @return _OK if copy success, otherwise error code.
            virtual ErrorCode Copy(Buffer *dst, const Buffer *src, void *command_queue = nullptr) = 0;

            // @brief Transfer memory from Host to Device
            // @return _OK if copy success, otherwise error code.
            virtual ErrorCode CopyToDevice(Buffer *dst, const Buffer *src, void *command_queue = nullptr) = 0;

            // @brief Transfer memory from Device to Host
            // @return _OK if copy success, otherwise error code.
            virtual ErrorCode CopyFromDevice(Buffer *dst, const Buffer *src, void *command_queue = nullptr) = 0;

            // @brief get factory device type
            DeviceType GetDeviceType();

        protected: // protected member,子类能访问
            DeviceType device_type_;
        };

        // @brief GetGlobalDeviceMap device type map
        std::map<DeviceType, std::shared_ptr<AbstractDevice>> &GetGlobalDeviceMap();

        // @brief Get Device
        AbstractDevice *GetDevice(DeviceType type);

        // @brief TypeDeviceRegister construct register device
        template <typename T>
        class TypeDeviceRegister
        {
        public:
            explicit TypeDeviceRegister(DeviceType type)
            {
                auto &device_map = GetGlobalDeviceMap();
                if (device_map.find(type) == device_map.end())
                {
                    device_map[type] = std::shared_ptr<T>(new T(type));
                }
            }
        };

    } // namespace device
} // namespace rayshape

#endif