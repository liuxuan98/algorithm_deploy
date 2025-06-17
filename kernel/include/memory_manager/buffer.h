
#ifndef _BUFFER_H_
#define _BUFFER_H_

#include "base/common.h"
#include "base/error.h"


namespace rayshape
{

    typedef struct RSMemoryInfo {
        MemoryType men_type = MEM_TYPE_NONE;
        DataType data_type = DATA_TYPE_NONE;
        unsigned int size = 0;
    } RSMemoryInfo;

    typedef struct RSMemoryData {
        unsigned int data_id;
        void *data_ptr = nullptr;
        void *context = nullptr;
    } RSMemoryData;

    typedef struct RSMemory {
        RSMemoryInfo mem_info;
        RSMemoryData mem_data;
    } RSMemory;

    class RS_PUBLIC Buffer {
    public:
        Buffer();

        Buffer(size_t size, MemoryType mem_type); // default uint8 datatype

        Buffer(const RSMemoryInfo &mem_info);

        Buffer(void *data, size_t size, MemoryType mem_type);

        Buffer(void *data, const RSMemoryInfo &mem_info);

        Buffer(unsigned int id, const RSMemoryInfo &mem_info);

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

        Buffer &operator=(const Buffer &buffer) = delete;

    private:
        // 设备对象
        // AbstractDevice *device_ = nullptr;
        // 内存结构体
        RSMemory mem_{ { MEM_TYPE_NONE, DATA_TYPE_NONE, 0 }, { 0, nullptr, nullptr } };
        // 内外部内存flag
        bool is_external_ = false;
    };

    RS_PUBLIC void *RSBufferDataGet(Buffer *buffer);

} // namespace rayshape

#endif