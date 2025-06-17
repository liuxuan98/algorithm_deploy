#ifndef _ABSTRACT_ALGORITHM_H_
#define _ABSTRACT_ALGORITHM_H_

namespace rayshape
{

    class AbstractAlgorithm {
    public:
        virtual void Init() = 0;
        virtual void DeInit() = 0;

        virtual void Reset() = 0;
    };

} // namespace rayshape

#endif