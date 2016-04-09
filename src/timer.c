#include <time.h>
#include <stdlib.h>
#include "timer.h"

/* ### NOTICE ### */
/* change the parameters according to the requirement */

/* set the granularity to 20ms */
#define GRANU 20

#define RNG_TV1 0x100
#define RNG_TV2 0x4000
#define RNG_TV3 0x100000
#define RNG_TV4 0x4000000

#define MSK_TV1 0xFF
#define MSK_TV2 0x3FFF
#define MSK_TV3 0xFFFFF
#define MSK_TV4 0x3FFFFFF
#define MSK_TV5 0xFFFFFFFF

#define NUM_TV 5

PRIVATE const uint16_t SIZE_TV[5] = {
	256, 64, 64, 64, 64
};

PRIVATE INLINE tstamp_t GetCurrentMillisec() {
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

PRIVATE inline TimeVector TimeVector_init(TimeVector tv, uint16_t size) {
	tv->cursor = 0;
	tv->vec = (DLinkList)malloc(sizeof(dlink_list_t) * size);
	uint16_t i;
	for(i = 0; i < size; ++i) {
		DLinkList_init(tv->vec + i);
	}
	return tv;
}

PRIVATE inline void TimeVector_close(TimeVector tv, uint16_t size) {
	uint16_t i;
	for(i = 0; i < size; ++i) {
		DLinkList_free(tv->vec + i, NULL);
	}
	free(tv->vec);
}

PRIVATE void TimeCurator_cascade(TimeCurator tc, DLinkList ret, tstamp_t now, uint8_t index) {
	TimeVector tv = tc->tvs + index;

	if(++tv->cursor >= SIZE_TV[index]) {
		tv->cursor = 0;
		if(index + 1 < NUM_TV) {
			TimeCurator_cascade(tc, ret, now, index + 1);
		}
	}
	DLinkList li     = tv->vec + tv->cursor;

	tstamp_t  latest = tc->latest;
	TimeSign  sign   = NULL;

	size_t len = DLinkList_length(li);
	for(size_t i = 0; i < len; ++i) {
		sign = (TimeSign)Element_getPtr(DLinkList_get(li), time_sign_t, link);
		/* put expired event to the result list instead of calling TimeCurator_checkInSign again */
		if(now >= sign->expire) {
			DLinkList_put(ret, &sign->link);
		} else {
			TimeCurator_checkInSign(tc, sign);
		}
	}
}

TimeCurator TimeCurator_init(TimeCurator tc) {
	uint8_t i;
	for(i = 0; i < 5; ++i) {
		TimeVector_init(tc->tvs + i, SIZE_TV[i]);
	}
	/* save the time when initializing */
	tc->latest = GetCurrentMillisec();
}

void TimeCurator_close(TimeCurator tc) {
	uint8_t i;
	for(i = 0; i < 5; ++i) {
		TimeVector_close(tc->tvs + i, SIZE_TV[i]);
	}
}

void TimeCurator_checkInSign(TimeCurator tc, TimeSign ts) {
	TimeVector ptv = tc->tvs;
	uint32_t stride = (ts->expire - tc->latest) / GRANU;

	DLinkList target;
	uint32_t index;
	if(stride < RNG_TV1) {
		index  = (stride + ptv[0].cursor) & 0xFF;
		target = ptv[0].vec + index;
	} else if(stride < RNG_TV2) {
		index  = ((stride >> 8) + ptv[1].cursor) & 0x3F;
		target = ptv[1].vec + index;
	} else if(stride < RNG_TV3) {
		index  = ((stride >> 14) + ptv[2].cursor) & 0x3F;
		target = ptv[2].vec + index;
	} else if(stride < RNG_TV4) {
		index = ((stride >> 20) + ptv[3].cursor) & 0x3F;
		target = ptv[3].vec + index;
	} else {
		index = ((stride >> 26) + ptv[4].cursor) & 0x3F;
		target = ptv[4].vec + index;
	}
	DLinkList_put(target, &ts->link);
}

void TimeCurator_checkOutSign(TimeCurator tc, DLinkList li) {
	tstamp_t now    = GetCurrentMillisec();
	uint32_t stride = (now - tc->latest) / GRANU;
	TimeVector tv   = tc->tvs;
	uint32_t i;
	for(i = 0; i < stride; ++i) {
		DLinkList current = tv->vec + tv->cursor;
		if(DLinkList_length(current)) {
			DLinkList_append(li, current);
		}

		tc->latest += GRANU;
		if(++(tv->cursor) >= SIZE_TV[0]) {
			tv->cursor = 0;
			// TODO bottleneck of performance
			TimeCurator_cascade(tc, li, now, 1);
		}
	}
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
typedef struct tevent {
	time_sign_t sign;
	int order;
} tevent_t;
int main() {
	tstamp_t ts;
	tstamp_t te;
	tevent_t tes[10];
	time_curator_t tc;
	TimeCurator_init(&tc);
	ts = GetCurrentMillisec();
	for(int i = 0; i < 10; ++i) {
		tes[i].order = i + 1;
		tes[i].sign.expire = ts + 2000 * (i + 1);
		TimeCurator_checkInSign(&tc, &tes[i].sign);
	}

	dlink_list_t res;
	DLinkList_init(&res);
	tevent_t* pts;
	size_t len;
	while(1) {
		TimeCurator_checkOutSign(&tc, &res);
		len = DLinkList_length(&res);
		for(int i = 0; i < len; ++i) {
			pts = (tevent_t*)Element_getPtr(DLinkList_get(&res), time_sign_t, link);
			printf("DELTA[%d] | ORDER[%d], EXPIRE[%lu]\n", (GetCurrentMillisec() - ts), pts->order, pts->sign.expire);
			pts->sign.expire += 2000 * pts->order;
			TimeCurator_checkInSign(&tc, &pts->sign);
		}
		usleep(20000);
	}
	return 0;
}
