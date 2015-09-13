#include "time.h"
#include <stdlib.h>
#include <string.h>

static time_t TIME_DIFF;

void time_module_init() {
	time_t local = time(NULL);
	struct tm tmGM;
	gmtime_r(&local, &tmGM);
	time_t utc = mktime(&tmGM);
	TIME_DIFF = utc - local;
}

time_t time_parse_from(const char* tstr) {
	struct tm time;
	char temp[26];
	temp[25] = '\0';
	strncpy(temp, tstr, 25);
	strptime(temp, FORMAT_NTZ, &time);
	time_t origin = mktime(&time);
	if(strcmp(tstr + 26, "GMT") == 0) {
		origin -= TIME_DIFF;
	}
	return origin;
}

void time_format_gmt(time_t* time, char* res) {
	struct tm gmtime;
	gmtime_r(time, &gmtime);
	strftime(res, 30, FORMAT_GMT, &gmtime);
}

void time_format_loc(time_t* time, char* res) {
	struct tm loctime;
	localtime_r(time, &loctime);
	strftime(res, 30, FORMAT_LOC, &loctime);
}

int time_get_day(time_t* time) {
	struct tm loc;
	localtime_r(time, &loc);
	return loc.tm_mday;
}

int time_get_month(time_t* time) {
	struct tm loc;
	localtime_r(time, &loc);
	return loc.tm_mon + 1;
}

int time_get_year(time_t* time) {
	struct tm loc;
	localtime_r(time, &loc);
	return loc.tm_year + 1900;
}
