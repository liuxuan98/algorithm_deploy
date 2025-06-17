#ifndef _CPU_DEVICE_H_
#define _CPU_DEVICE_H_

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

            virtual ~CpuDevice();

        public:
            virtual ErrorCode Allocate(size_t size, void **ptr);

            virtual ErrorCode Free(void *ptr);

            virtual ErrorCode Copy(void *src, void *dst, size_t size,
                                   void *command_queue = nullptr);

            virtual ErrorCode CopyToDevice(void *src, void *dst, size_t size,
                                           void *command_queue = nullptr);

            virtual ErrorCode CopyFromDevice(void *src, void *dst, size_t size,
                                             void *command_queue = nullptr);

            virtual ErrorCode Copy(Buffer *dst, const Buffer *src, void *command_queue = nullptr);

            virtual ErrorCode CopyToDevice(Buffer *dst, const Buffer *src,
                                           void *command_queue = nullptr);

            virtual ErrorCode CopyFromDevice(Buffer *dst, const Buffer *src,
                                             void *command_queue = nullptr);
        };
    } // namespace device
} // namespace rayshape

#endif