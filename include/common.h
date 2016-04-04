#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdint.h>

#define PRIVATE static
#define INLINE __inline__ __attribute__((always_inline))

#if __GNUC__ >= 3
	#define LIKELY(x) __builtin_expect(!!(x), 1)
	#define UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
	#define LIKELY(x) (x)
	#define UNLIKELY(x) (x)
#endif


#endif //_COMMON_H_
