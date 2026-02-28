#ifndef _CODEC_ENGINE_H_
#define _CODEC_ENGINE_H_

#include "utils/codec/codec_include.h"

namespace rayshape
{
    class CodecEngine {
    public:
        CodecEngine() = default;
        ~CodecEngine() = default;

        CodecEngine(const CodecEngine &) = delete;
        CodecEngine &operator=(const CodecEngine &) = delete;

    public:
        virtual std::string Encode(const std::string &) const = 0;
        virtual void Encode(const std::string &plain, std::string &cipher) const = 0;
        virtual std::string Decode(const std::string &) const = 0;
        virtual void Decode(const std::string &cipher, std::string &plain) const = 0;

    protected:
        friend class cereal::access;

        template <typename Archive> void serialize(Archive &ar, std::uint32_t const version) {}
    };
} // namespace rayshape

#endif // _CODEC_ENGINE_H_