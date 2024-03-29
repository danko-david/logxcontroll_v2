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
//https://stackoverflow.com/questions/3826832/is-there-a-portable-way-to-print-a-message-from-the-c-preprocessor

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
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <dlfcn.h>//TODO POSIX supports dlopen, windows has LoadLibrary function for this job

//if novaprova included
#ifdef INCLUDE_NOVAPROVA
	#include <np.h>
#endif

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


#ifdef LXC_EMBED_MODULE_EXPERIMENT
	#include "experiment/experiment.h"
#endif


/********************** Support specific build entities ***********************/

#if __STDC_VERSION__ >= 201101L

	//#ifdef VERBOSE_BUILD
		#pragma message "build target: C11"
	//#endif

#endif

#if _WIN32
   //define something for Windows (32-bit and 64-bit, this part is common)
	#ifdef VERBOSE_BUILD
		#pragma message "build target: Windows 32"
	#endif
   #ifdef _WIN64
		#ifdef VERBOSE_BUILD
			#pragma message "build target: Windows 64"
		#endif
      //define something for Windows (64-bit only)
   #endif
#elif __APPLE__
    #include "TargetConditionals.h"
    #if TARGET_IPHONE_SIMULATOR
		#ifdef VERBOSE_BUILD
			#pragma message "build target: IOS simulator"
		#endif
         // iOS Simulator
    #elif TARGET_OS_IPHONE
		#ifdef VERBOSE_BUILD
			#pragma message "build target: Iphone"
		#endif
        // iOS device
    #elif TARGET_OS_MAC
        // Other kinds of Mac OS
		#ifdef VERBOSE_BUILD
			#pragma message "build target: Mac OS"
		#endif
    #else
    #   error "Unknown Apple platform"
    #endif
#elif __linux__
	#ifdef VERBOSE_BUILD
		#pragma message "build target: linux"
	#endif

#elif __unix__ // all unices not caught above
	#ifdef VERBOSE_BUILD
		#pragma message "build target: Unix like system"
	#endif
    // Unix
#elif defined(_POSIX_VERSION)
	#ifdef VERBOSE_BUILD
		#pragma message "build target: POSIX compatible"
	#endif
    // POSIX
#else
#   error "Unknown compiler"
#endif

/************************** Tester functions/ macros **************************/

#ifdef INCLUDE_NOVAPROVA
	#define TEST_ASSERT_EQUAL NP_ASSERT_EQUAL
	#define TEST_ASSERT_PTR_EQUAL NP_ASSERT_PTR_EQUAL
	#define TEST_ASSERT_NOT_NULL NP_ASSERT_NOT_NULL
	#define TEST_ASSERT_TRUE NP_ASSERT_TRUE
	#define TEST_ASSERT_NOT_EQUAL NP_ASSERT_NOT_EQUAL
#elif EMBEDDED_TESTER /*Another tester infrastructure.*/

#else	/*No tester infrastructure*/
	#define TEST_ASSERT_EQUAL
	#define TEST_ASSERT_PTR_EQUAL
	#define TEST_ASSERT_NOT_NULL
	#define TEST_ASSERT_TRUE
	#define TEST_ASSERT_NOT_EQUAL
#endif


/************************* Common build specific types ************************/

/**
 * to fully utilize this function use -rdynamic linking option
 * */
void print_stack_trace();

void print_stack_trace_then_terminalte();



#include "pthread.h"
#define short_lock pthread_spinlock_t
#define long_lock pthread_mutex_t

struct condition_wait
{
	pthread_mutex_t mutex;
	pthread_cond_t condition;
};

struct conditional_wait;

//POSIX thread vs green threads
#ifdef USE_GREEN_THREAD
	#include "pth.h"
	typedef pth_t* thread_handle;
	//#include "core/external/github/geofft/vireo/vireo.h"
	//	typedef vireo_entry* thread_handle;

	#define short_lock pth_mutex_t
	#define long_lock pth_mutex_t

	struct conditional_wait
	{
		pth_mutex_t mutex;
		pth_cond_t condition;
	};

#else
	#include "pthread.h"
	typedef pthread_t* thread_handle;

	#define short_lock pthread_spinlock_t
	#define long_lock pthread_mutex_t

	struct conditional_wait
	{
		pthread_mutex_t mutex;
		pthread_cond_t condition;
	};
#endif /*end of pthread vs green thread */

int lxc_thread_start
(
	thread_handle*,
	void (*executor)(void*),
	void* param
);
int lxc_thread_setup_after_start();

int lxc_thread_init_env(void);
int lxc_thread_destroy_env(void);

void lxc_thread_cond_wait_notify(struct conditional_wait* cw);
void lxc_thread_cond_wait_wait(struct conditional_wait* cw);

int lxc_thread_cond_wait_lock(struct conditional_wait* cw);
int lxc_thread_cond_wait_unlock(struct conditional_wait* cw);
int lxc_thread_cond_wait_trylock(struct conditional_wait* cw);

int lxc_thread_cond_wait_destroy(struct conditional_wait* cw);


#define MAX_CONCURRENCY_RETRY 300

int short_lock_init(short_lock*);

/**
 * returns:
 *	0: if successfully locked,
 *	EBUSY: if a thread already holds the lock
 *	other: use lxc_fetch_error
 * */
int short_lock_lock(short_lock*);

/**
 * returns:
 *	0: if successfully locked,
 *	EBUSY: if a thread already holds the lock
 *	other: use lxc_fetch_error
 * */
int short_lock_trylock(short_lock*);
int short_lock_unlock(short_lock*);
int short_lock_destroy(short_lock*);



int long_lock_init(long_lock*);
int long_lock_lock(long_lock*);

/**
 * returns:
 *	0: if successfully locked,
 *	EBUSY: if a thread already holds the lock
 *	other: use lxc_fetch_error
 * */
int long_lock_trylock(long_lock*);
int long_lock_unlock(long_lock*);
int long_lock_destroy(long_lock*);




int c_sleep(int);

int c_usleep(useconds_t);



#endif /* BUILD_H_ */
