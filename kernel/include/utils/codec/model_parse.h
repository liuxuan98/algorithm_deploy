#ifndef _MODEL_PARSE_H_
#define _MODEL_PARSE_H_

#include "model/model.h"
#include "utils/codec/model_codec.h"
#include "base/macros.h"

#include <fstream>
#include <sstream>

namespace rayshape
{
    /**
     * @brief Parse model from file
     * @param[in] filename Path to serialized model file
     * @return std::unique_ptr<Model> Parsed model object
     */
    RS_PUBLIC std::unique_ptr<Model> ParseModel(const std::string &filename);

    /**
     * @brief Parse model from memory buffer
     * @param[in] model_buf Pointer to model data buffer
     * @param[in] size Size of the model data buffer
     * @return std::unique_ptr<Model> Parsed model object
     */
    RS_PUBLIC std::unique_ptr<Model> ParseModel(const char *model_buf, std::size_t size);

    /**
     * @brief Parse encrypted model from file
     * @param[in] filename Path to encrypted serialized model file
     * @param[in] password Password for decryption
     * @return std::unique_ptr<Model> Parsed model object
     */
    RS_PUBLIC std::unique_ptr<Model> ParseEncryptedModel(const std::string &filename,
                                                         const std::string &password);

    /**
     * @brief Parse encrypted model from memory buffer
     * @param[in] model_buf Pointer to encrypted model data buffer
     * @param[in] size Size of the encrypted model data buffer
     * @param[in] password Password for decryption
     * @return std::unique_ptr<Model> Parsed model object
     */
    RS_PUBLIC std::unique_ptr<Model> ParseEncryptedModel(const char *model_buf, std::size_t size,
                                                         const std::string &password);
} // namespace rayshape

#endif // _MODEL_PARSE_H_
