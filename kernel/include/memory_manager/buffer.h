/**
 * @file buffer.h
 * @brief buffer内存管理模块
 * @copyright .
 *
 *
 * @author Liuxuan
 * @email liuxuan@rayshape.com
 * @date 2025-05-16
 * @version 1.0.0
 */

#ifndef BUFFER_H
#define BUFFER_H

#include "base/common.h"
#include "base/error.h"

namespace rayshape
{

    typedef struct RSMemoryInfo {
        MemoryType mem_type_ = MemoryType::NONE;
        DataType data_type_ = DataType::NONE;
        unsigned int size_ = 0;
    } RSMemoryInfo;

    typedef struct RSMemoryData {
        unsigned int data_id_ = 0;
        void *data_ptr_ = nullptr;
        void *context_ = nullptr;
    } RSMemoryData;

    typedef struct RSMemory {
        RSMemoryInfo mem_info_;
        RSMemoryData mem_data_;
    } RSMemory;

    class RS_PUBLIC Buffer {
    public:
        Buffer();

        Buffer(size_t size, MemoryType mem_type); // default uint8 datatype

        Buffer(const RSMemoryInfo &mem_info);

        Buffer(void *data, size_t size, MemoryType mem_type);

        Buffer(void *data, const RSMemoryInfo &mem_info);

        Buffer(unsigned int id, const RSMemoryInfo &mem_info);

        Buffer(const Buffer &) = delete;
        Buffer &operator=(const Buffer &) = delete;

        Buffer(Buffer &&) = delete;
        Buffer &operator=(Buffer &&) = delete;

        virtual ~Buffer();

        static Buffer *Alloc(size_t size, MemoryType mem_type);

        static Buffer *Alloc(const RSMemoryInfo &mem_info);

        static Buffer *Create(void *data, size_t size, MemoryType mem_type);

        static Buffer *Create(void *data, const RSMemoryInfo &mem_info);

        static Buffer *Create(unsigned int id, const RSMemoryInfo &mem_info);

        MemoryType GetMemoryType() const;

        RSMemoryInfo GetMemoryInfo() const;

        void *GetDataPtr() const;

        unsigned int GetDataId() const;

        size_t GetDataSize() const;

        ErrorCode DeepCopy(Buffer &dst);

        bool GetExternalFlag() const;

    private:
        ErrorCode Malloc(const RSMemoryInfo &mem_info, RSMemoryData &mem_data);

        // void Init(const RSMemoryInfo &mem_info, const RSMemoryData &mem_data, bool external);

        void Init(const RSMemoryInfo &mem_info, void *data, unsigned int data_id, bool external);

    private:
        // 设备对象
        // AbstractDevice *device_ = nullptr;
        // 内存结构体
        RSMemory mem_;
        // 内外部内存flag
        bool is_external_ = false;
    };

    RS_PUBLIC void *RSBufferDataGet(Buffer *buffer);

} // namespace rayshape

#endif // BUFFER_H
