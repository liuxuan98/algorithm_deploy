#include "inference/tensorrt/tensorrt_timing_cache.h"

#if NV_TENSORRT_MAJOR > 7

#include <iostream>
#include <sstream>
#include <fstream>
#include <stdexcept>

namespace rayshape
{
    namespace tensorrt
    {
        //! \brief RAII object that locks the specified file.
        //!
        //! The FileLock class uses a lock file to specify that the
        //! current file is being used by a TensorRT tool or sample
        //! so that things like the TimingCache can be updated across
        //! processes without having conflicts.
        class FileLock {
        public:
            FileLock(nvinfer1::ILogger &logger, std::string const &fileName);
            ~FileLock();
            FileLock() = delete;                            // no default ctor
            FileLock(FileLock const &) = delete;            // no copy ctor
            FileLock &operator=(FileLock const &) = delete; // no copy assignment
            FileLock(FileLock &&) = delete;                 // no move ctor
            FileLock &operator=(FileLock &&) = delete;      // no move assignment

        private:
            //!
            //! The logger that emits any error messages that might show up.
            //!
            nvinfer1::ILogger &mLogger;

            //!
            //! The filename that the FileLock is protecting from multiple
            //! TensorRT processes from writing to.
            //!
            std::string const mFileName;

#ifdef _MSC_VER
            //!
            //! The file handle on windows for the file lock.
            //!
            HANDLE mHandle{};
#else
            //!
            //! The file handle on linux for the file lock.
            //!
            FILE *mHandle{};
            //!
            //! The file descriptor on linux of the file lock.
            //!
            int32_t mDescriptor{-1};
#endif
        }; // class FileLock

        FileLock::FileLock(nvinfer1::ILogger &logger, std::string const &fileName) :
            mLogger(logger), mFileName(fileName) {
            std::string lockFileName = mFileName + ".lock";
#ifdef _MSC_VER
            {
                std::stringstream ss;
                ss << "Trying to set exclusive file lock " << lockFileName << std::endl;
                mLogger.log(nvinfer1::ILogger::Severity::kVERBOSE, ss.str().c_str());
            }
            // MS docs said this is a blocking IO if "FILE_FLAG_OVERLAPPED" is not provided
            mHandle =
                CreateFileA(lockFileName.c_str(), GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, 0, NULL);
            if (mHandle == INVALID_HANDLE_VALUE) {
                throw std::runtime_error("Failed to lock " + lockFileName + "!");
            }
#elif defined(__QNX__)
            // Calling lockf(F_TLOCK) on QNX returns -1; the reported error is 89 (function not
            // implemented).
#else
            mHandle = fopen(lockFileName.c_str(), "wb+");
            if (mHandle == nullptr) {
                throw std::runtime_error("Cannot open " + lockFileName + "!");
            }
            {
                std::stringstream ss;
                ss << "Trying to set exclusive file lock " << lockFileName << std::endl;
                mLogger.log(nvinfer1::ILogger::Severity::kVERBOSE, ss.str().c_str());
            }
            mDescriptor = fileno(mHandle);
            auto ret = lockf(mDescriptor, F_LOCK, 0);
            if (ret != 0) {
                mDescriptor = -1;
                fclose(mHandle);
                throw std::runtime_error("Failed to lock " + lockFileName + "!");
            }
#endif
        }

        FileLock::~FileLock() {
            std::string lockFileName = mFileName + ".lock";
#ifdef _MSC_VER
            if (mHandle != INVALID_HANDLE_VALUE) {
                CloseHandle(mHandle);
            }
#elif defined(__QNX__)
            // Calling lockf(F_TLOCK) on QNX returns -1; the reported error is 89 (function not
            // implemented).
#else
            if (mDescriptor != -1) {
                auto ret = lockf(mDescriptor, F_ULOCK, 0);
                if (mHandle != nullptr) {
                    fclose(mHandle);
                }
                if (ret != 0) {
                    std::stringstream ss;
                    ss << "Failed to unlock " << lockFileName << ", please remove " << lockFileName
                       << ".lock manually!" << std::endl;
                    mLogger.log(nvinfer1::ILogger::Severity::kVERBOSE, ss.str().c_str());
                }
            }
#endif
        }

        ErrorCode LoadTimingCacheFile(nvinfer1::ILogger &logger, std::string const &inFileName,
                                      std::vector<char> &content) {
            try {
                FileLock fileLock{logger, inFileName};
                std::ifstream iFile(inFileName, std::ios::in | std::ios::binary);
                if (!iFile) {
                    std::stringstream ss;
                    ss << "Could not read timing cache from: " << inFileName
                       << ". A new timing cache will be generated and written.";
                    logger.log(nvinfer1::ILogger::Severity::kWARNING, ss.str().c_str());
                    return ErrorCode::RS_INVALID_FILE;
                }
                iFile.seekg(0, std::ifstream::end);
                size_t fsize = iFile.tellg();
                iFile.seekg(0, std::ifstream::beg);
                content.resize(fsize);
                iFile.read(content.data(), fsize);
                iFile.close();
                std::stringstream ss;
                ss << "Loaded " << fsize << " bytes of timing cache from " << inFileName;
                logger.log(nvinfer1::ILogger::Severity::kINFO, ss.str().c_str());
                return ErrorCode::RS_SUCCESS;
            } catch (std::exception const &e) {
                RS_LOGE("Exception while loading timing cache file %s: %s\n", inFileName, e.what());
                return ErrorCode::RS_INVALID_FILE;
            }
        }

        std::unique_ptr<nvinfer1::ITimingCache> BuildTimingCacheFromFile(
            nvinfer1::ILogger &logger, nvinfer1::IBuilderConfig &config,
            std::string const &timingCacheFile) {
            std::unique_ptr<nvinfer1::ITimingCache> timingCache{};
            std::vector<char> timingCacheContents;
            ErrorCode err = LoadTimingCacheFile(logger, timingCacheFile, timingCacheContents);
            if (err != ErrorCode::RS_SUCCESS) {
                return nullptr;
            }

            timingCache.reset(
                config.createTimingCache(timingCacheContents.data(), timingCacheContents.size()));
            if (timingCache == nullptr) {
                logger.log(nvinfer1::ILogger::Severity::kERROR,
                           ("Failed to create ITimingCache from file " + timingCacheFile).c_str());
                return nullptr;
            }

            config.clearFlag(nvinfer1::BuilderFlag::kDISABLE_TIMING_CACHE);
            if (!config.setTimingCache(*timingCache, true)) {
                logger.log(nvinfer1::ILogger::Severity::kERROR,
                           ("IBuilderConfig#setTimingCache failed with timing cache from file "
                            + timingCacheFile)
                               .c_str());
                return nullptr;
            }
            return timingCache;
        }

        ErrorCode SaveTimingCacheFile(nvinfer1::ILogger &logger, std::string const &outFileName,
                                      nvinfer1::IHostMemory const *blob) {
            try {
                FileLock fileLock{logger, outFileName};
                std::ofstream oFile(outFileName, std::ios::out | std::ios::binary);
                if (!oFile) {
                    std::stringstream ss;
                    ss << "Could not write timing cache to: " << outFileName;
                    logger.log(nvinfer1::ILogger::Severity::kWARNING, ss.str().c_str());
                    return ErrorCode::RS_INVALID_FILE;
                }
                oFile.write(reinterpret_cast<char const *>(blob->data()), blob->size());
                oFile.close();
                std::stringstream ss;
                ss << "Saved " << blob->size() << " bytes of timing cache to " << outFileName;
                logger.log(nvinfer1::ILogger::Severity::kINFO, ss.str().c_str());
                return ErrorCode::RS_SUCCESS;
            } catch (std::exception const &e) {
                RS_LOGE("Exception while saving timing cache file %s: %s\n", outFileName, e.what());
                return ErrorCode::RS_INVALID_FILE;
            }
        }

        ErrorCode UpdateTimingCacheFile(nvinfer1::ILogger &logger, std::string const &fileName,
                                        nvinfer1::ITimingCache const *timingCache,
                                        nvinfer1::IBuilder &builder) {
            try {
                std::unique_ptr<nvinfer1::IBuilderConfig> config{builder.createBuilderConfig()};
                std::vector<char> timingCacheContents;
                ErrorCode err = LoadTimingCacheFile(logger, fileName, timingCacheContents);
                if (err != ErrorCode::RS_SUCCESS) {
                    return err;
                }
                std::unique_ptr<nvinfer1::ITimingCache> fileTimingCache{config->createTimingCache(
                    timingCacheContents.data(), timingCacheContents.size())};

                fileTimingCache->combine(*timingCache, false);
                std::unique_ptr<nvinfer1::IHostMemory> blob{fileTimingCache->serialize()};
                if (!blob) {
                    throw std::runtime_error("Failed to serialize combined ITimingCache!");
                }

                FileLock fileLock{logger, fileName};
                std::ofstream oFile(fileName, std::ios::out | std::ios::binary);
                if (!oFile) {
                    std::stringstream ss;
                    ss << "Could not write timing cache to: " << fileName;
                    logger.log(nvinfer1::ILogger::Severity::kWARNING, ss.str().c_str());
                    return ErrorCode::RS_INVALID_FILE;
                }

                oFile.write(reinterpret_cast<char const *>(blob->data()), blob->size());
                oFile.close();
                std::stringstream ss;
                ss << "Saved " << blob->size() << " bytes of timing cache to " << fileName;
                logger.log(nvinfer1::ILogger::Severity::kINFO, ss.str().c_str());
                return ErrorCode::RS_SUCCESS;
            } catch (std::exception const &e) {
                RS_LOGE("Exception while updating timing cache file %s: %s\n", fileName, e.what());
                return ErrorCode::RS_INVALID_FILE;
            }
        }
    } // namespace tensorrt
} // namespace rayshape

#endif
