#include "datetime.h"
#include <stdlib.h>
#include <string.h>

#define FORMAT_NTZ "%a, %d %b %Y %H:%M:%S"
#define FORMAT_GMT "%a, %d %b %Y %H:%M:%S GMT"
#define FORMAT_LOC "%a, %d %b %Y %H:%M:%S LOC"

#define PRIVATE static

PRIVATE inline time_t TIME_DIFF() {
	time_t local = time(NULL);
	struct tm tmGM;
	gmtime_r(&local, &tmGM);
	time_t utc = mktime(&tmGM);
	return utc - local;
}

time_t DateTime_parseFrom(const char* tstr) {
	struct tm time;
	char temp[26];
	temp[25] = '\0';
	strncpy(temp, tstr, 25);
	strptime(temp, FORMAT_NTZ, &time);
	time_t origin = mktime(&time);
	if(strcmp(tstr + 26, "GMT") == 0) {
		origin -= TIME_DIFF();
	}
	return origin;
}

void DateTime_formatGmt(time_t* time, char* res) {
	struct tm gmtime;
	gmtime_r(time, &gmtime);
	strftime(res, 30, FORMAT_GMT, &gmtime);
}

void DateTime_formatLoc(time_t* time, char* res) {
	struct tm loctime;
	localtime_r(time, &loctime);
	strftime(res, 30, FORMAT_LOC, &loctime);
}

int DateTime_getDay(time_t* time) {
	struct tm loc;
	localtime_r(time, &loc);
	return loc.tm_mday;
}

int DateTime_getMonth(time_t* time) {
	struct tm loc;
	localtime_r(time, &loc);
	return loc.tm_mon + 1;
}

int DateTime_getYear(time_t* time) {
	struct tm loc;
	localtime_r(time, &loc);
	return loc.tm_year + 1900;
}
