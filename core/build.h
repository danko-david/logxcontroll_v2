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


#include <dlfcn.h>//TODO POSIX supoports dlopen, windows has LoadLibrary function for this job
/*
 * Required types:
 * 	- atomic_pointer_t
 * 		bool compare_and_swap(&atomic_pointer_type, pointer_old, pointer_new);
 *
 * 	- short_lock_t
 * 		short_lock_init(&short_lock);
 * 		short_lock_lock(&short_lock);
 * 		short_lock_unlock(&short_lock);
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

#ifdef LXC_EMBED_MODULE_ARITHMETIC
	#include "arithmetic/arithmetic.h"
#endif

#ifdef LXC_EMBED_MODULE_POSIX
	#include "posix/liblxc_posix.h"
#endif


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

	//bool compare_and_set(type );





#elif __unix__ // all unices not caught above
    // Unix
#elif defined(_POSIX_VERSION)
    // POSIX
#else
#   error "Unknown compiler"
#endif





#endif /* PLATFORM_H_ */
