#ifndef LIMIT_H
#define LIMIT_H

#include <limits.h>

#if __GNUC__
  #if __x86_64__ || __ppc64__
	// #define MININT  INT_MIN
	// #define MAXINT  INT_MAX
	// typedef long	var_t;
	#define MININT  SHRT_MIN
	#define MAXINT  SHRT_MAX
	typedef int		var_t;
  #else
	#define MININT  SHRT_MIN
	#define MAXINT  SHRT_MAX
	typedef int		var_t;
  #endif
#endif

#endif