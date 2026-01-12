#ifndef CPU_DEVICE_H
#define CPU_DEVICE_H

#include "device/abstract_device.h"

namespace rayshape
{
    namespace device
    {
        /**
         * @brief cpu device,finish cpu memory management operation.
         * @details it is not public interface.
         *
         */

        class RS_PUBLIC CpuDevice: public AbstractDevice {
        public:
            CpuDevice(DeviceType device_type);

            ~CpuDevice() override;

        public:
            ErrorCode Allocate(size_t size, void **ptr) override;

            ErrorCode Free(void *ptr) override;

            ErrorCode Copy(const void *src, void *dst, size_t size,
                           void *command_queue = nullptr) override;

            ErrorCode CopyToDevice(const void *src, void *dst, size_t size,
                                   void *command_queue = nullptr) override;

            ErrorCode CopyFromDevice(const void *src, void *dst, size_t size,
                                     void *command_queue = nullptr) override;

            ErrorCode Copy(const Buffer *src, Buffer *dst, void *command_queue = nullptr) override;

            ErrorCode CopyToDevice(const Buffer *src, Buffer *dst,
                                   void *command_queue = nullptr) override;

            ErrorCode CopyFromDevice(const Buffer *src, Buffer *dst,
                                     void *command_queue = nullptr) override;
        };
    } // namespace device
} // namespace rayshape

#endif // CPU_DEVICE_H
