#ifndef _TIME_H_
#define _TIME_H_

#define _XOPEN_SOURCE
#include <time.h>

#define FORMAT_NTZ "%a, %d %b %Y %H:%M:%S"
#define FORMAT_GMT "%a, %d %b %Y %H:%M:%S GMT"
#define FORMAT_LOC "%a, %d %b %Y %H:%M:%S LOC"

void time_module_init(void);

void time_format_gmt(time_t*, char*);

void time_format_loc(time_t*, char*);

time_t time_parse_from(const char*);

int time_get_day(time_t*);

int time_get_month(time_t*);

int time_get_year(time_t*);

#endif // _TIME_H_
