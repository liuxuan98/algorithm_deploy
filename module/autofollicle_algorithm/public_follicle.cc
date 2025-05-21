#include "autofollicle.h"
#include "error.h"
// sample
RS_API ErrorCode FollicleAlgorthmInit(const char *model_json, void **algor_handle)
{
    ErrorCode ret = RS_SUCCESS;
    Follicle *handle = new Follicle(); // 转换一下

    ret = handle->Init(const char *models_json);
    if (ret != RS_SUCCESS)
    {
        delete handle;
        handle = nullptr;
        // 打印日志
        return ret
    }
    *algor_handle = static_cast<void *>(handle);
    // LOGI();
    return ret;
}
// 可以考虑做一个结构体,包含原图指针，图片高宽等信息
RS_API ErrorCode FollicleAlgorthmProcess(void *algor_handle, unsigned char *src_image_ptr, FollicleOutParams *out_params)
{
    ErrorCode ret = RS_SUCCESS;
    Follicle *handle = static_cast<Follicle *>(algor_handle);

    ret = handle->Process(unsigned char *src_image_ptr, FollicleOutParams *out_params);
    if (ret != RS_SUCCESS)
    {
        // 打印日志
        delete handle; // 调用算法对象的析构
        return ret
    }

    return RS_SUCCESS;
}

RS_API void FollicleAlgorthmRelease(void *algor_handle)
{
    Follicle *handle = static_cast<Follicle *>(algor_handle);
    delete handle;
    handle = nullptr;
    // LOGI(); 算法释放成功
}
