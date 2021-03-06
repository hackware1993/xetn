#ifndef _TIME_H_
#define _TIME_H_

#define _XOPEN_SOURCE
#include <time.h>

#define DATETIME_LEN 30
#define TIMESTAMP_LEN 20

void DateTime_formatGmt(time_t*, char*);
void DateTime_formatLoc(time_t*, char*);
void DateTime_formatTimeStamp(time_t*, char*);

time_t DateTime_parseFrom(const char*);

int DateTime_getDay(time_t*);
int DateTime_getMonth(time_t*);
int DateTime_getYear(time_t*);

#endif // _TIME_H_
