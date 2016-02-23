#ifndef _SYSINFO_H_
#define _SYSINFO_H_

#include <unistd.h>
#include <limits.h>

#define SysInfo_getHostId()   gethostid()
#define SysInfo_getCpuNum()   sysconf(_SC_NPROCESSORS_ONLN)
#define SysInfo_getPageSize() getpagesize()
#define SysInfo_getMaxPath()  sysconf(_POSIX_PATH_MAX)
#define SysInfo_getMaxName()  sysconf(_POSIX_NAME_MAX)

#endif // _SYSINFO_H_
