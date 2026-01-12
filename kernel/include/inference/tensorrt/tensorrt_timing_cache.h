#ifndef _TENSORRT_TIMING_CACHE_H_
#define _TENSORRT_TIMING_CACHE_H_

#include "base/error.h"
#include "inference/tensorrt/tensorrt_include.h"

#if NV_TENSORRT_MAJOR > 7

#ifdef _MSC_VER
// Needed so that the max/min definitions in windows.h do not conflict with std::max/min.
#define NOMINMAX
#include <windows.h>
#undef NOMINMAX
#else
#include <stdio.h>  // fileno
#include <unistd.h> // lockf
#endif
#include <string>
#include <iosfwd>
#include <memory>
#include <vector>

namespace rayshape
{
    namespace tensorrt
    {
        //! \brief Loads the binary contents of a timing cache file into a char vector.
        //!
        //! \note This is a blocking operation, as this method will acquire an exclusive file lock
        //! on the timing cache file for the duration of the read. \returns The binary data from the
        //! file, or an empty vector if an error occurred.
        ErrorCode LoadTimingCacheFile(nvinfer1::ILogger &logger, std::string const &inFileName,
                                      std::vector<char> &content);

        //! \brief Helper method to load a timing cache from a file, build an ITimingCache with the
        //! data, and then set the new timing cache to the builder config. If the file is blank, or
        //! cannot be read, a new timing cache will be created from scratch.
        //!
        //! \returns The newly created timing cache, or nullptr if an error occured during creation.
        std::unique_ptr<nvinfer1::ITimingCache> BuildTimingCacheFromFile(
            nvinfer1::ILogger &logger, nvinfer1::IBuilderConfig &config,
            std::string const &timingCacheFile);

        //! \brief Saves the contents of a timing cache to a binary file.
        //!
        //! \note This is a blocking operation, as this method will acquire an exclusive file lock
        //! on the timing cache file for the duration of the write.
        ErrorCode SaveTimingCacheFile(nvinfer1::ILogger &logger, std::string const &outFileName,
                                      nvinfer1::IHostMemory const *blob);

        //! \brief Updates the contents of a timing cache binary file.
        //! This operation loads the timing cache file, combines it with the passed timingCache, and
        //! serializes the combined timing cache.
        //!
        //! \note This is a blocking operation, as this method will acquire an exclusive file lock
        //! on the timing cache file for the duration of the write.
        ErrorCode UpdateTimingCacheFile(nvinfer1::ILogger &logger, std::string const &fileName,
                                        nvinfer1::ITimingCache const *timingCache,
                                        nvinfer1::IBuilder &builder);
    } // namespace tensorrt
} // namespace rayshape

#endif

#endif