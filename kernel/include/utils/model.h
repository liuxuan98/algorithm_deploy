// 实现模型文件的读取 转到内存中的功能类//
#ifndef _MODEL_H_
#define _MODEL_H_

#include "base/common.h"
#include "base/error.h"

namespace rayshape
{
    // typedef struct TarFile
    // {
    //     char *data // file context

    //         std::string filename; // file name

    //     size_t size; // file size
    // }

    class Model {
    public:
        // 解析模型成为内存文件

        ErrorCode Parse(const std::string &filename,
                        ModelType model_type); // 完成模型文件的解析(解密)
                                               // ,依据文件数加载到内存中,注意区分不同推理框架的模型

        // 是否需要匹配license的标志位和模型时候解析完成并且初始化成功的标志位,
    private:
        std::string filename_; // 加密的模型文件路径

        std::vector<std::string> parse_filename_;
    };
} // namespace rayshape
#endif