#include <algorithm>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <thread>
#include <vector>

// 3th_party

// develop
#include "utils/time_profile.h"

namespace rayshape
{
    namespace utils
    {

#ifdef RS_ENABLE_TIME_PROFILE

        using std::chrono::duration_cast;
        using std::chrono::microseconds;

        typedef enum
        {
            PROCEDURE_FLAG_START = 0,
            PROCEDURE_FLAG_END,
        } ProcedureFlag;

        struct ProcedureTimePoint
        {
            std::string name_device_; // default name_host_
            std::string name_procedure_;
            ProcedureFlag flag_;

            std::chrono::time_point<std::chrono::system_clock> tp_;

            ProcedureTimePoint(std::string name_procedure, ProcedureFlag flag) : name_procedure_(name_procedure), flag_(flag){};
            ProcedureTimePoint(std::string name_device, std::string name_procedure, ProcedureFlag flag)
                : name_device_(name_device), name_procedure_(name_procedure), flag_(flag){};
        };

        struct ProcedureElapsed
        {
            std::string name_device_; // default name_host_
            std::string name_procedure_;
            int level_;
            int count_;
            int order_;
            float sum_elapsed_;

            ProcedureElapsed(
                std::string name_device, std::string name_procedure, int level, int count, int order, float sum_elapsed)
                : name_device_(name_device),
                  name_procedure_(name_procedure),
                  level_(level),
                  count_(count),
                  order_(order),
                  sum_elapsed_(sum_elapsed){};

            friend bool operator<(ProcedureElapsed &lhs, ProcedureElapsed &rhs);
        };

        bool operator<(ProcedureElapsed &lhs, ProcedureElapsed &rhs)
        {
            if (lhs.order_ < rhs.order_)
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        // placeholder
        std::string GetCompiler()
        {
            std::string compiler_info = {};
#ifdef _WIN32
            compiler_info = "MSVC";
#elif __linux__
            compiler_info = "GCC";
#else
            compiler_info = "CLANG";
#endif
            return compiler_info;
        }

        std::string GetOS()
        {
            std::string os_info = {};
#ifdef _WIN32
            os_info = "Windows";
#elif __ANDROID__
            os_info = "Android";
#elif __APPLE__
#if defined(TARGET_OS_OSX)
            os_info = "Macos";
#elif TARGET_OS_IPHONE
            os_info = "iOS";
#else
            os_info = "other Apple os";
#endif
#elif __linux__
            os_info = "Linux";
#else
            os_info = "Not set"
#endif
            return os_info;
        }

        std::string GetHost()
        {
            return "Not set";
        }

        long GetRemainMemorySize()
        {
            return static_cast<long>(-1);
        }

        class TimeProfile
        {
        public:
            // TimeProfile(std::string name_compiler, std::string name_os,
            //             std::string name_host)
            //     : name_compiler_(name_compiler),
            //       name_os_(name_os),
            //       name_host_(name_host) {
            //   remain_memory_size_ = GetRemainMemorySize();
            // };
            static TimeProfile *getInstance()
            {
                if (p == NULL)
                    p = new TimeProfile();
                return p;
            }
            ~TimeProfile();
            // override
            void InsertProcedureTimePoint(std::string, ProcedureFlag flag);
            void InsertProcedureTimePoint(std::string, std::string, ProcedureFlag flag);

            int DownloadTimeProfile(std::string name_file);

        private:
        private:
            TimeProfile();
            static TimeProfile *p;
            void UpdataProcedureElapsedMap(std::map<std::string, std::shared_ptr<ProcedureElapsed>> &time_elapsed,
                                           std::shared_ptr<ProcedureTimePoint> &p_tp_start,
                                           std::shared_ptr<ProcedureTimePoint> &p_tp_end,
                                           std::string full_name,
                                           int level,
                                           int order);

            void Write2TimeLogCSV(std::string name_file, std::vector<std::shared_ptr<ProcedureElapsed>> &p_e_vec);

            std::string name_compiler_;
            std::string name_os_;
            std::string name_host_;
            long remain_memory_size_;

            std::vector<std::shared_ptr<ProcedureTimePoint>> p_tp_vec_;
            int order_ = 0;
        };
        TimeProfile *TimeProfile::p = nullptr;

        TimeProfile::TimeProfile()
        {
            name_compiler_ = GetCompiler();
            name_os_ = GetOS();
            name_host_ = GetHost();
            remain_memory_size_ = GetRemainMemorySize();
        }

        TimeProfile::~TimeProfile()
        {
            ;
        }

        void TimeProfile::InsertProcedureTimePoint(std::string name_procedure, ProcedureFlag flag)
        {
            std::chrono::time_point<std::chrono::system_clock> tp = std::chrono::system_clock::now();
            auto p_tp = std::make_shared<ProcedureTimePoint>(name_procedure, flag);
            p_tp->tp_ = tp;
            p_tp_vec_.push_back(p_tp);
        }

        void TimeProfile::InsertProcedureTimePoint(std::string name_device, std::string name_procedure, ProcedureFlag flag)
        {
            std::chrono::time_point<std::chrono::system_clock> tp = std::chrono::system_clock::now();
            auto p_tp = std::make_shared<ProcedureTimePoint>(name_device, name_procedure, flag);
            p_tp->tp_ = tp;
            p_tp_vec_.push_back(p_tp);
        }

        void TimeProfile::UpdataProcedureElapsedMap(std::map<std::string, std::shared_ptr<ProcedureElapsed>> &time_elapsed,
                                                    std::shared_ptr<ProcedureTimePoint> &p_tp_start,
                                                    std::shared_ptr<ProcedureTimePoint> &p_tp_end,
                                                    std::string full_name,
                                                    int level,
                                                    int order)
        {
            float delta = duration_cast<microseconds>(p_tp_end->tp_ - p_tp_start->tp_).count() / 1000.0f;

            if (time_elapsed.find(full_name) == time_elapsed.end())
            {
                std::string name_device;
                if (p_tp_start->name_device_.empty())
                {
                    name_device = name_host_;
                }
                else
                {
                    name_device = p_tp_start->name_device_;
                }
                auto p_e = std::make_shared<ProcedureElapsed>(name_device, p_tp_start->name_procedure_, level, 1, order, delta);
                time_elapsed.insert(std::pair<std::string, std::shared_ptr<ProcedureElapsed>>(full_name, p_e));
            }
            else
            {
                time_elapsed[full_name]->sum_elapsed_ += delta;
                ++time_elapsed[full_name]->count_;
            }
        }

        void TimeProfile::Write2TimeLogCSV(std::string name_file, std::vector<std::shared_ptr<ProcedureElapsed>> &p_e_vec)
        {
            time_t timep;
            time(&timep);
            char dairy[200];

            strftime(dairy, sizeof(dairy), "%Y-%m-%d-%H_%M_%S", localtime(&timep));
            std::string full_name = name_file + "_" + "time_profile_" + dairy + ".csv";
            // if directory not exist, create it
            std::string file_parent_dir = fileUtils::GetFileParentPath(full_name, false);
            fileUtils::MakeDirectory(file_parent_dir);

            std::ofstream outfile(full_name, std::ios::out);
            if (outfile.is_open())
            {
                outfile.setf(std::ios::fixed);
                outfile.width(12);
                outfile.precision(6);

                // background
                outfile << "compiler"
                        << "," << name_compiler_.c_str() << std::endl;
                outfile << "os"
                        << "," << name_os_.c_str() << std::endl;
                outfile << "host"
                        << "," << name_host_.c_str() << std::endl;
                outfile << "remain memory size"
                        << "," << remain_memory_size_ << "byte" << std::endl;
                outfile << std::endl;

                // elapsed
                outfile << "procedure name"
                        << ","
                        << "avg elapsed (ms)"
                        << ","
                        << "sum elapsed (ms)"
                        << ","
                        << "count"
                        << ","
                        << "device" << std::endl;
                for (auto ite : p_e_vec)
                {
                    std::string full_procedure = "#";
                    for (int i = 1; i < ite->level_; ++i)
                    {
                        full_procedure += "#";
                    }
                    full_procedure += (" " + ite->name_procedure_);
                    outfile << full_procedure << ",";
                    float avg_elapsed = ite->sum_elapsed_ / ite->count_;
                    outfile << avg_elapsed << ",";
                    outfile << ite->sum_elapsed_ << ",";
                    outfile << ite->count_ << ",";
                    outfile << ite->name_device_ << std::endl;
                }
            }

            outfile.close();
        }

        int TimeProfile::DownloadTimeProfile(std::string name_file)
        {
            // pass == match (start and end)
            std::map<std::string, int> order_map;
            std::map<std::string, std::shared_ptr<ProcedureElapsed>> time_elapsed;
            std::vector<std::shared_ptr<ProcedureTimePoint>> match_vec;
            for (auto ite_p_tp : p_tp_vec_)
            {
                int level = 0;
                std::string full_name = "map_/";
                for (auto match : match_vec)
                {
                    ++level;
                    full_name += (match->name_procedure_ + "/");
                    if (order_map.find(full_name) == order_map.end())
                    {
                        order_map.insert(std::pair<std::string, int>(full_name, order_));
                        ++order_;
                    }
                }
                if (PROCEDURE_FLAG_START == ite_p_tp->flag_)
                {
                    match_vec.push_back(ite_p_tp);
                }
                else
                {
                    auto start = match_vec.back();
                    if (start->name_procedure_ == ite_p_tp->name_procedure_)
                    {
                        UpdataProcedureElapsedMap(time_elapsed, start, ite_p_tp, full_name, level, order_map[full_name]);
                        match_vec.pop_back();
                    }
                    else
                    {
                        // TODO
                        std::cout << "[error!] start and end can't match.";
                        return -1;
                    }
                }
            }

            // sort
            std::vector<std::shared_ptr<ProcedureElapsed>> p_e_vec(time_elapsed.size());
            int i = 0;
            for (auto ite_src : time_elapsed)
            {
                p_e_vec[i] = ite_src.second;
                i++;
            }
            struct
            {
                bool operator()(std::shared_ptr<ProcedureElapsed> lhs, std::shared_ptr<ProcedureElapsed> rhs) const
                {
                    return lhs->order_ < rhs->order_;
                }
            } CustomLess;
            std::sort(p_e_vec.begin(), p_e_vec.end(), CustomLess);

            // csv log
            Write2TimeLogCSV(name_file, p_e_vec);

            return 0;
        }

        // std::shared_ptr<TimeProfile> time_profile=nullptr;
        TimeProfile *time_profile = TimeProfile::getInstance();

        // thread_local

        // extern
        std::string GenProcedureName(std::string name, int line)
        {
            return (name + "_" + std::to_string(line));
        }

        // void InitTimeProfile() {
        //     time_profile = std::make_shared<TimeProfile>();
        // }

        // void InitTimeProfile(std::string name_compiler, std::string name_os, std::string name_host) {
        //     time_profile = std::make_shared<TimeProfile>(name_compiler, name_os, name_host);
        // }

        void InsertProcedureTimePointStart(std::string name_procedure)
        {
            time_profile->InsertProcedureTimePoint(name_procedure, PROCEDURE_FLAG_START);
        }

        void InsertProcedureTimePointStart(std::string name_device, std::string name_procedure)
        {
            time_profile->InsertProcedureTimePoint(name_device, name_procedure, PROCEDURE_FLAG_START);
        }

        void InsertProcedureTimePointEnd(std::string name_procedure)
        {
            time_profile->InsertProcedureTimePoint(name_procedure, PROCEDURE_FLAG_END);
        }

        void InsertProcedureTimePointEnd(std::string name_device, std::string name_procedure)
        {
            time_profile->InsertProcedureTimePoint(name_device, name_procedure, PROCEDURE_FLAG_END);
        }

        int ExternDownloadTimeProfile(std::string name_file)
        {
            return time_profile->DownloadTimeProfile(name_file);
        }

#endif

    } // namespace utils
} // namespace rayshape