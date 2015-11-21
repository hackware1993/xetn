#ifndef _OPTIMIZE_H_
#define _OPTIMIZE_H_

#if __GNUC__ >= 3
	#define LIKELY(x) __builtin_expect(!!(x), 1)
	#define UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
	#define LIKELY(x) (x)
	#define UNLIKELY(x) (x)
#endif


#endif // _OPTIMIZE_H_
