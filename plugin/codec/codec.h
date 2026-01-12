#ifndef _RAYSHAPE_CODEC_H_
#define _RAYSHAPE_CODEC_H_

#include "dag/node.h"

// CV 解码节点模块
namespace rayshape
{
    // init 接口中构建图？
    // process接口中传数据指针在接口内部传递为cv::Mat最后
    class Decode: public dag::Node {
    public:
        // Decode(); 去掉默认构造函数
        // 如果父类没有默认构造函数(无参构造函数),派生类的构造函数必须显示调用父类的带参数构造函数，如果不显示调用，编译器会尝试调用父类的默认构造函数,由于不存在,会编译报错.

        Decode(const std::string &name); // 构造函数

        Decode(const std::string &name, std::vector<dag::Edge *> inputs,
               std::vector<dag::Edge *> outputs); // 构造函数

        virtual ~Decode();

        virtual ErrorCode Run() = 0;

    private:
    };

} // namespace rayshape

#endif
