#include "dag/edge/data_package.h"

namespace rayshape
{
    namespace dag
    {
        DataPackage::DataPackage() {}

        DataPackage::~DataPackage() {
            Destory();
        }

        ErrorCode DataPackage::SetBuff(Buffer *buffer, bool is_external) {
            ErrorCode ret = RS_SUCCESS;
            if (buffer != data_anything_) {
                Destory();
            }
            is_external_ = is_external;
            written_ = true;

            data_anything_ = (void *)buffer;
            data_deleter_ = [](void *d) { delete static_cast<Buffer *>(d); };
            type_info_ = const_cast<std::type_info *>(&typeid(Buffer));

            return ret;
        }

        Buffer *DataPackage::CreateBuff(device::AbstractDevice *device, RSMemoryInfo mem_info) {
            Buffer *buffer = nullptr;

            if (data_anything_ == nullptr) {
                buffer = Buffer::Alloc(mem_info);
            } else {
                Destory();
                buffer = Buffer::Alloc(mem_info);
            }
            is_external_ = false;

            written_ = true;
            data_anything_ = (void *)buffer;

            data_deleter_ = [](void *d) { delete static_cast<Buffer *>(d); };
            type_info_ = const_cast<std::type_info *>(&typeid(Buffer));

            return buffer;
        }

        bool DataPackage::NotifyWrite(Buffer *buffer) {
            if ((void *)buffer == data_anything_) {
                written_ = true;
                return true;
            } else {
                return false;
            }
        }

        Buffer *DataPackage::GetBuff() {
            return static_cast<Buffer *>(data_anything_);
        }

#ifdef ENABLE_3RD_OPENCV
        ErrorCode DataPackage::SetMat(cv::Mat *mat, bool is_external) {
            ErrorCode ret = RS_SUCCESS;
            if (mat != data_anything_) {
                Destory();
            }
            is_external_ = is_external;
            written_ = true;
            data_anything_ = (void *)mat;

            data_deleter_ = [](void *d) { delete static_cast<cv::Mat *>(d); };
            type_info_ = const_cast<std::type_info *>(&typeid(cv::Mat));
            return ret;
        }

        cv::Mat *DataPackage::CreateMat(int rows, int cols, int type, const cv::Scalar &value) {
            cv::Mat *mat = nullptr;
            if (data_anything_ == nullptr) {
                mat = new cv::Mat(rows, cols, type, value);
            } else {
                Destory();
                mat = new cv::Mat(rows, cols, type, value);
            }
            is_external_ = false;
            written_ = false;
            data_anything_ = (void *)mat;

            data_deleter_ = [](void *d) { delete static_cast<cv::Mat *>(d); };
            type_info_ = const_cast<std::type_info *>(&typeid(cv::Mat));

            return mat;
        }

        bool DataPackage::NotifyWrite(cv::Mat *mat) {
            if ((void *)mat == data_anything_) {
                written_ = true;
                return true;
            } else {
                return false;
            }
        }

        cv::Mat *DataPackage::GetMat() {
            return static_cast<cv::Mat *>(data_anything_);
        }

#endif

        /**
         *
         * @brief 数据包销毁函数
         */
        void DataPackage::Destory() {
            if (!is_external_ && data_anything_ != nullptr) {
                data_deleter_(data_anything_);
            }
            is_external_ = true;
            data_anything_ = nullptr;
            type_info_ = nullptr;
            data_deleter_ = nullptr;
        }

        template <typename T>
        ErrorCode DataPackage::SetAny(T *t, bool is_external = true) {
            ErrorCode ret = RS_SUCCESS;
            if (data_anything_ == nullptr) {
                data_anything_ = (void *)(t);
            } else {
            }

            return RS_SUCCESS;
        }

        template <typename T, typename... Args>
        T *DataPackage::CreateAny(int index, Args &&...args) {
            T *t = nullptr;
            if (data_anything_ == nullptr) {
                t = new T(std::forward<Args>(args)...);
            } else {
                destory();
                t = new T(std::forward<Args>(args)...);
            }
            if (t == nullptr) {
                RS_LOGE("Failed to create T type data.\n");
                return nullptr;
            }
            is_external_ = false;
            data_anything_ = (void *)(t);
            type_info_ = const_cast<std::type_info *>(&typeid(T));

            data_deleter_ = [](void *d) { delete static_cast<T *>(d); };

            return t;
        }

        template <typename T>
        T *DataPackage::GetAny() {
            if (typeid(T) != *type_info_) {
                RS_LOGE("set and get type not match!\n");
                return nullptr;
            }

            return static_cast<T *>(data_anything_);
        }

        void DataPackage::SetIndex(int64_t index) {
            index_ = index;
        }

        int64_t DataPackage::GetIndex() {
            return index_;
        }

        // pipeline datapackage.
        PipelineDataPackage::PipelineDataPackage(int consumers_size) :
            DataPackage(), consumers_size_(consumers_size), consumers_count_(0) {}

        PipelineDataPackage::~PipelineDataPackage() {
            consumers_size_ = 0;
            consumers_count_ = 0;
        }

        ErrorCode PipelineDataPackage::SetBuff(Buffer *buffer, bool is_external) {
            std::unique_lock<std::mutex> lock(mutex_);
            ErrorCode ret = DataPackage::SetBuff(buffer, is_external);
            RS_RETURN_ON_NEQ(ret, RS_SUCCESS, "DataPackage::set failed!\n");
            cv_.notify_all();
            return ret;
        }

        bool PipelineDataPackage::NotifyWrite(Buffer *buffer) {
            std::unique_lock<std::mutex> lock(mutex_);
            bool status = DataPackage::NotifyWrite(buffer);
            if (status) {
                cv_.notify_all();
            }
            return status;
        }

        Buffer *PipelineDataPackage::GetBuff() {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait(lock, [this] { return written_; });
            return DataPackage::GetBuff();
        }

        Buffer *PipelineDataPackage::GetBufferDirect() {
            return DataPackage::GetBuff();
        }

#ifdef ENABLE_3RD_OPENCV
        ErrorCode PipelineDataPackage::SetMat(cv::Mat *mat, bool is_external) {
            std::unique_lock<std::mutex> lock(mutex_);
            ErrorCode ret = DataPackage::SetMat(mat, is_external);
            RS_RETURN_ON_NEQ(ret, RS_SUCCESS, "DataPackage SetMat Failed.\n");
            cv_.notify_all();
            return ret;
        }

        bool PipelineDataPackage::NotifyWrite(cv::Mat *mat) {
            std::unique_lock<std::mutex> lock(mutex_);
            bool status = DataPackage::NotifyWrite(mat);
            if (status) {
                cv_.notify_all();
            }
            return status;
        }
        cv::Mat *PipelineDataPackage::GetMat() {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait(lock, [this] { return written_; });
            return DataPackage::GetMat();
        }

        cv::Mat *PipelineDataPackage::GetMatDirect() {
            return DataPackage::GetMat();
        }

#endif

        ErrorCode PipelineDataPackage::TakeDataPackage(DataPackage *package) {
            std::unique_lock<std::mutex> lock(mutex_);
            // ErrorCode ret =;
            // TODO 获取外部数据包信息到本个数据包

            return RS_SUCCESS;
        }

        void PipelineDataPackage::IncreaseConsumersSize() {
            std::unique_lock<std::mutex> lock(mutex_);
            consumers_size_++;
        }

        void PipelineDataPackage::IncreaseConsumersCount() {
            std::unique_lock<std::mutex> lock(mutex_);
            consumers_count_++;
        }

        int PipelineDataPackage::GetConsumersSize() {
            return consumers_size_;
        }

        int PipelineDataPackage::GetConsumersCount() {
            return consumers_count_;
        }

    } // namespace dag
} // namespace rayshape