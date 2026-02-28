/*
  *  time profile
    liuxuan 2025.04.16
*/
#ifndef TIME_PROFILE_H
#define TIME_PROFILE_H

#include "base/macros.h"

namespace rayshape
{
    namespace utils
    {
#ifdef ENABLE_TIME_PROFILER

        extern RS_PUBLIC void InsertProcedureTimePointStart(std::string name_procedure);

        extern RS_PUBLIC void InsertProcedureTimePointStart(std::string name_device,
                                                            std::string name_procedure);

        extern RS_PUBLIC void InsertProcedureTimePointEnd(std::string name_procedure);

        extern RS_PUBLIC void InsertProcedureTimePointEnd(std::string name_device,
                                                          std::string name_procedure);

        extern RS_PUBLIC int ExternDownloadTimeProfile(std::string name_file);

#define GEN_PROCEDURE_NAME(s) GenProcedureName(s, __LINE__)

#define INSERT_TIME_POINT_START InsertProcedureTimePointStart

#define INSERT_TIME_POINT_END InsertProcedureTimePointEnd

#define DOWNLOAD_TIME_PROFILE ExternDownloadTimeProfile

#else
#define GEN_PROCEDURE_NAME(s)

#define INSERT_TIME_POINT_START(d, s)

#define INSERT_TIME_POINT_END(d, s)

#define DOWNLOAD_TIME_PROFILE(s)

#endif
    } // namespace utils
} // namespace rayshape

#endif // TIME_PROFILE_H
