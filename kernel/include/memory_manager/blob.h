/**
 * @file blob.h
 * @brief 模型输入输出传输blob管道模块
 * @copyright (c) 2025 Shenzhen RayShape Medical Technology Co., Ltd.
 *
 *
 * @author Liuxuan
 * @email liuxuan@rayshape.com
 * @date 2025-05-16
 * @version 1.0.0
 */

#ifndef _BLOB_H_
#define _BLOB_H_

#include "base/common.h"
#include "base/macros.h"
#include "base/error.h"
#include "memory_manager/buffer.h"

namespace rayshape
{

#define MAX_DIMS_SIZE 6
#define MAX_BLOB_NAME 64

    typedef struct Dims {
        int size;
        int value[MAX_DIMS_SIZE];
    } Dims;

    typedef struct RS_PUBLIC Blob {
        DeviceType device_type = DeviceType::NONE;
        // device_type describes device type cpu, gpu,.........

        DataType data_type = DataType::NONE;
        // data_type describes data precision fp32,int8,..........

        DataFormat data_format = DataFormat::AUTO;
        // data format describes data layout NCHW,NHWC,CHW,HWC,HW,.........

        Dims dims = {}; // Dims describes data dims.

        char name[MAX_BLOB_NAME + 1] = {}; // blob name

        Buffer *buffer = nullptr; // buffer blob data buffer,share_ptr manage?

    } Blob;

    using BlobMap = std::map<std::string, Blob *>; // BlobMap

    using ShapeMap = std::map<std::string, Dims *>; // ShapeMap

    /**
     * @brief create a new blob by link information
     * @param[in] device_type blob device type
     * @param[in] data_type blob data type
     * @param[in] data_format blob data format
     * @param[in] name blob name
     * @param[in] dims blob dims
     * @return Blob *
     */
    RS_PUBLIC Blob *BlobAlloc(DeviceType device_type, DataType data_type, DataFormat data_format,
                              const char *name, const Dims *dims); // 分配一个blob

    /**
     * @brief make a new blob by reference a data
     * @param[in] device_type blob device type
     * @param[in] data_type blob data type
     * @param[in] data_format blob data format
     * @param[in] name blob name
     * @param[in] dims blob dims
     * @param[in] blob_data data pointer
     * @return Blob *
     */
    RS_PUBLIC Blob *BlobMake(DeviceType device_type, DataType data_type, DataFormat data_format,
                             const char *name, const Dims *dims, void *blob_data);

    /**
     * @brief get a blob data size information
     * @param[in] blob blob pointer
     * @return size_t blob size
     */
    RS_PUBLIC size_t BlobSizeGet(const Blob *blob);

    /**
     * @brief between two same condition blob memory copy
     * @param[in] src_blob src blob pointer
     * @param[in] dst_blob dst blob pointer
     * @return ErrorCode RS_SUCCESS if copy success, otherwise error code
     */
    RS_PUBLIC ErrorCode BlobCopy(const Blob *src_blob, Blob *dst_blob);

    /**
     * @brief free a blob
     * @param[in] blob blob pointer
     * @return ErrorCode RS_SUCCESS if copy success, otherwise error code
     */
    RS_PUBLIC ErrorCode BlobFree(Blob *blob);
} // namespace rayshape

#endif
