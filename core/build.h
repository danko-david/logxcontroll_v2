/*
 * platform.h
 *
 *  Created on: 2016.05.26.
 *      Author: szupervigyor
 */

#include "core/logxcontroll.h"

#ifndef BUILD_H_
#define BUILD_H_

//http://stackoverflow.com/questions/5919996/how-to-detect-reliably-mac-os-x-ios-linux-windows-in-c-preprocessor

/**
 * Required supports:
 * 	- standard C89 library
 * 	- atomic variable support
 * 	- mutex/monitor support (long_lock_t)
 * 	- spinlock support (short_lock_t)
 * 	- thread: start/join/exit/notify
 *
 * */

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "pthread.h"

#include <dlfcn.h>//TODO POSIX supports dlopen, windows has LoadLibrary function for this job


#define STATIC_ASSERT(COND,MSG) typedef char static_assertion_##MSG[(!!(COND))*2-1]

// token pasting madness:
#define COMPILE_TIME_ASSERT3(X,L) STATIC_ASSERT(X,static_assertion_at_line_##L)
#define COMPILE_TIME_ASSERT2(X,L) COMPILE_TIME_ASSERT3(X,L)
#define COMPILE_TIME_ASSERT(X)    COMPILE_TIME_ASSERT2(X,__LINE__)

#ifndef UNUSED
	#define UNUSED(x) (void)(x)
#endif

//
void lxc_on_bug_found(void);

/*
 * Required types:
 * 	- atomic_pointer_t
 * 		bool compare_and_swap(&atomic_pointer_type, pointer_old, pointer_new);
 *
 * 	- short_lock_t
 * 		short_lock_init(&short_lock);
 * 		short_lock_lock(&short_lock);
 * 		short_lock_unlock(&short_lock);
 * 		short_lock_fetch_error(int, char*)
 * 	- long_lock_t
 *		-||-
 *
 * */

//comment this out if you doesn't want to print out negative refcount values.
#define DEBUG_FOR_NEGATIVE_REFCOUNT

void lxc_load_embedded_modules
(
	const char** errors,
	int maxlength
);
/*
Available embeddable modules macro:
#define LXC_EMBED_MODULE_ARITHMETIC
#define LXC_EMBED_MODULE_POSIX
*/



/*************************** Embedded modules *********************************/

#ifdef LXC_EMBED_MODULE_ARITHMETIC
	#include "arithmetic/arithmetic.h"
#endif

#ifdef LXC_EMBED_MODULE_POSIX
	#include "posix/liblxc_posix.h"
#endif


/********************** Support specific build entities ***********************/


#if __STDC_VERSION__ >= 201101L



#elif _WIN32
   //define something for Windows (32-bit and 64-bit, this part is common)
   #ifdef _WIN64
      //define something for Windows (64-bit only)
   #endif
#elif __APPLE__
    #include "TargetConditionals.h"
    #if TARGET_IPHONE_SIMULATOR
         // iOS Simulator
    #elif TARGET_OS_IPHONE
        // iOS device
    #elif TARGET_OS_MAC
        // Other kinds of Mac OS
    #else
    #   error "Unknown Apple platform"
    #endif
#elif __linux__

	#define short_lock_t pthread_spinlock_t
	#define long_lock_t pthread_mutex_t

	struct condition_wait_t
	{
		pthread_mutex_t mutex;
		pthread_cond_t condition;
	};


#elif __unix__ // all unices not caught above
    // Unix
#elif defined(_POSIX_VERSION)
    // POSIX
#else
#   error "Unknown compiler"
#endif




/************************* Common build specific types ************************/

int short_lock_init(short_lock_t*);

/**
 * returns:
 *	0: if successfully locked,
 *	EBUSY: if a thread already holds the lock
 *	other: use lxc_fetch_error
 * */
int short_lock_lock(short_lock_t*);

/**
 * returns:
 *	0: if successfully locked,
 *	EBUSY: if a thread already holds the lock
 *	other: use lxc_fetch_error
 * */
int short_lock_trylock(short_lock_t*);
int short_lock_unlock(short_lock_t*);
int short_lock_destroy(short_lock_t*);



int long_lock_init(long_lock_t*);
int long_lock_lock(long_lock_t*);

/**
 * returns:
 *	0: if successfully locked,
 *	EBUSY: if a thread already holds the lock
 *	other: use lxc_fetch_error
 * */
int long_lock_trylock(long_lock_t*);
int long_lock_unlock(long_lock_t*);
int long_lock_destroy(long_lock_t*);

#endif /* BUILD_H_ */
