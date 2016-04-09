#ifndef _TIMER_H_
#define _TIMER_H_

#include "common.h"
#include "util/list.h"

typedef uint64_t tstamp_t;

typedef struct time_sign {
	tstamp_t expire;
	dlink_t  link;
} time_sign_t, *TimeSign;

typedef struct time_vector {
	DLinkList vec;
	/* DO NOT use uint8_t, can be overflow */
	uint16_t  cursor;
} time_vector_t, *TimeVector;

typedef struct time_curator {
	time_vector_t tvs[5];
	tstamp_t   latest;
} time_curator_t, *TimeCurator;

TimeCurator TimeCurator_init(TimeCurator);
void        TimeCurator_close(TimeCurator);
void        TimeCurator_checkInSign(TimeCurator, TimeSign);
void        TimeCurator_checkOutSign(TimeCurator, DLinkList);
void        TimeCurator_removeSign(TimeCurator, TimeSign);
#endif // _TIMER_H_
