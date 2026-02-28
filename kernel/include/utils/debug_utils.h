#ifndef _DEBUG_UTILS_H_
#define _DEBUG_UTILS_H_

#include "base/error.h"
#include "base/logger.h"

namespace rayshape
{
    namespace utils
    {
#define CHECK_RET(expr)                                                                            \
    {                                                                                              \
        ErrorCode ret = expr;                                                                      \
        if (ret != ErrorCode::RS_SUCCESS) {                                                        \
            RS_LOGE("%s failed: %d", #expr, ret);                                                  \
            return ret;                                                                            \
        }                                                                                          \
    }

        template <typename T> void print_vec(const std::vector<T> &vec, int n_line) {
            for (int i = 0; i < vec.size(); i++) {
                std::cout << vec[i];
                if (i != vec.size() - 1) {
                    if (i % n_line == n_line - 1) {
                        std::cout << std::endl;
                    } else {
                        std::cout << ", ";
                    }
                } else {
                    std::cout << std::endl;
                }
            }
        }
    } // namespace utils
} // namespace rayshape

#endif
