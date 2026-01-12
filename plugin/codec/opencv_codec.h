#ifndef _OPENCV_CODEC_H_
#define _OPENCV_CODEC_H_

#include "codec.h"

namespace rayshape
{
    class OpenCvImageDecode: public Decode {
    public:
        OpenCvImageDecode(const std::string &name);

        OpenCvImageDecode(const std::string &name, std::vector<dag::Edge *> inputs,
                          std::vector<dag::Edge *> outputs);

        ~OpenCvImageDecode();

        ErrorCode Run();
        // virtual ErrorCode Decode(const std::string &file_path, std::vector<uint8_t> &data)
        // override; virtual ErrorCode Encode(const std::string &file_path,
        //                          const std::vector<uint8_t> &data) override;
    private:
        std::string desc_;
    };
} // namespace rayshape

#endif