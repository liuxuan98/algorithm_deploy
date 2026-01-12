#ifndef _CUDA_DEVICE_H_
#define _CUDA_DEVICE_H_

#include "device/cuda/cuda_include.h"
#include "device/abstract_device.h"

namespace rayshape
{
    namespace device
    {
        /**
         * @brief cuda device,finish cuda memory management operation.
         * @details it is not public interface.
         *
         */

        class RS_PUBLIC CudaDevice: public AbstractDevice {
        public:
            CudaDevice(DeviceType device_type);

            virtual ~CudaDevice();

        public:
            virtual ErrorCode Allocate(size_t size, void **ptr);

            virtual ErrorCode Free(void *ptr);

            virtual ErrorCode Copy(const void *src, void *dst, size_t size,
                                   void *command_queue = nullptr);

            virtual ErrorCode CopyToDevice(const void *src, void *dst, size_t size,
                                           void *command_queue = nullptr);

            virtual ErrorCode CopyFromDevice(const void *src, void *dst, size_t size,
                                             void *command_queue = nullptr);

            virtual ErrorCode Copy(const Buffer *src, Buffer *dst, void *command_queue = nullptr);

            virtual ErrorCode CopyToDevice(const Buffer *src, Buffer *dst,
                                           void *command_queue = nullptr);

            virtual ErrorCode CopyFromDevice(const Buffer *src, Buffer *dst,
                                             void *command_queue = nullptr);
        };
    } // namespace device
} // namespace rayshape

#endif