/**
 * @file model.h
 * @brief 模型管理的抽象模块
 * @copyright .
 *
 * @author Liuxuan
 * @email liuxuan@rayshape.com
 * @date 2025-05-16
 * @version 1.0.0
 */

#ifndef MODEL_H
#define MODEL_H

#include "base/common.h"
#include "base/macros.h"

namespace rayshape
{
    class RS_PUBLIC Model {
    public:
        Model() = default;
        virtual ~Model();

        virtual ModelType GetModelType() const = 0;
    };
} // namespace rayshape

#endif // MODEL_H
