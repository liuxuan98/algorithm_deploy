#ifndef _ABSTRACT_ALGORITHM_H_
#define _ABSTRACT_ALGORITHM_H_
// 给module部分的算法,继承使用,也可以不用直接在module不考虑继承定义相关的类就行
namespace rayshape
{
    class AbstractAlgorithm
    {
    public:
        virtual void Init() = 0;
        virtual void DeInit() = 0;

        virtual void Reset() = 0;
    };

}

#endif