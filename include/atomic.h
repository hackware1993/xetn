#ifndef _ATOMIC_H_
#define _ATOMIC_H_

#include <stdint.h>

#define ATOMIC_CASB4(p, o, n) \
	__sync_bool_compare_and_swap((uint32_t*)(p), *(uint32_t*)&(o), *(uint32_t*)&(n))
#define ATOMIC_CASB8(p, o, n) \
	__sync_bool_compare_and_swap((uint64_t*)(p), *(uint64_t*)&(o), *(uint64_t*)&(n))
#define ATOMIC_CASB16(p, o, n) \
	__sync_bool_compare_and_swap((__uint128_t*)(p), *(__uint128_t*)&(o), *(__uint128_t*)&(n))
#define ATOMIC_CASV4(r, p, o, n) \
	*(uint32_t*)&(r) = __sync_val_compare_and_swap((uint32_t*)(p), *(uint32_t*)&(o), *(uint32_t*)&(n))
#define ATOMIC_CASV8(r, p, o, n) \
	*(uint64_t*)&(r) = __sync_val_compare_and_swap((uint64_t*)(p), *(uint64_t*)&(o), *(uint64_t*)&(n))
#define ATOMIC_CASV16(r, p, o, n) \
	*(__uint128_t*)&(r) = __sync_val_compare_and_swap((__uint128_t*)(p), *(__uint128_t*)&(o), *(__uint128_t*)&(n))

#endif // _ATOMIC_H_
