#ifndef _ABSTRACT_ALGORITHM_H_
#define ABSTRACT_ALGORITHM_H

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