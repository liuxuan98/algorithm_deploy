#ifndef _TENSORRT_COMMON_H_
#define _TENSORRT_COMMON_H_

namespace rayshape
{
    namespace tensorrt
    {
        struct TensorRTDeleter {
            template <typename T> void operator()(T *obj) const {
#if NV_TENSORRT_MAJOR > 7
                delete obj;
#else
                if (obj) {
                    obj->destroy();
                }
#endif
            }
        };

        template <typename T> using TrtUniquePtr = std::unique_ptr<T, TensorRTDeleter>;
    } // namespace tensorrt
} // namespace rayshape

#endif