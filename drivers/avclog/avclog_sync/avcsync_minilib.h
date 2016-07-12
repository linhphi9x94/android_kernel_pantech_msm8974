#ifndef MINILIB_H
#define MINILIB_H


//-----------------------------------------------------------------------------
#ifdef  __KERNEL__

#include <linux/stddef.h>
#include <linux/kernel.h>
#include <linux/slab.h>

#define MEM_ALLOC(n)  {kmalloc((n), (GFP_USER))} 
#define MEM_FREE  kfree
#define PRINTF  printk

#define NDEBUG
#ifdef NDEBUG
#define AVCLOG_ASSERT(f)  ((f) ? ((void*)0) : assert_fail(#f, __FILE__, __LINE__))
#else
#define AVCLOG_ASSERT(cond) \
	do { \
		if (unlikely(!(cond))) { \
			panic("avclog failed: %s:%d\n", \
			      __FILE__, __LINE__); \
		} \
	} while (0)

#endif

void assert_fail(const char* s, const char* fname, int fline);


//-----------------------------------------------------------------------------
#else  

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define MEM_ALLOC  malloc
#define MEM_FREE  free
#define PRINTF  printf
//#define ASSERT  assert
#define ASSERT  ((void)0)

//-----------------------------------------------------------------------------
#endif  // __KERNEL__

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE  0
#endif


//-----------------------------------------------------------------------------
#endif  // MINILIB_H
