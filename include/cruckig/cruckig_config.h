#ifndef CRUCKIG_CONFIG_H
#define CRUCKIG_CONFIG_H

/*
 * cruckig Performance Configuration
 *
 * These macros control optimization levels. They can be set via CMake options
 * or defined before including any cruckig header.
 *
 * CRUCKIG_OPT_LEVEL:
 *   0 = Default (portable, no special flags)
 *   1 = Optimized (LTO + O3)
 *   2 = Aggressive (LTO + O3 + march=native + fast-math)
 */

/* Compiler detection */
#if defined(__GNUC__) || defined(__clang__)
#  define CRUCKIG_GCC_COMPAT 1
#else
#  define CRUCKIG_GCC_COMPAT 0
#endif

/* Branch prediction hints */
#if CRUCKIG_GCC_COMPAT
#  define CRUCKIG_LIKELY(x)   __builtin_expect(!!(x), 1)
#  define CRUCKIG_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#  define CRUCKIG_LIKELY(x)   (x)
#  define CRUCKIG_UNLIKELY(x) (x)
#endif

/* Force inline for critical hot-path functions */
#if CRUCKIG_GCC_COMPAT
#  define CRUCKIG_FORCE_INLINE static inline __attribute__((always_inline))
#  define CRUCKIG_HOT __attribute__((hot))
#else
#  define CRUCKIG_FORCE_INLINE static inline
#  define CRUCKIG_HOT
#endif

/* restrict keyword (C99) */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#  define CRUCKIG_RESTRICT restrict
#elif CRUCKIG_GCC_COMPAT
#  define CRUCKIG_RESTRICT __restrict__
#elif defined(_MSC_VER)
#  define CRUCKIG_RESTRICT __restrict
#else
#  define CRUCKIG_RESTRICT
#endif

/* Prefetch hint for upcoming memory access */
#if CRUCKIG_GCC_COMPAT
#  define CRUCKIG_PREFETCH(addr) __builtin_prefetch(addr, 0, 1)
#else
#  define CRUCKIG_PREFETCH(addr) ((void)0)
#endif

#endif /* CRUCKIG_CONFIG_H */
