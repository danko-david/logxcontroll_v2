/*
 * build.c
 *
 *  Created on: 2016.05.26.
 *      Author: szupervigyor
 */

#include "core/logxcontroll.h"

//breakpoints
void lxc_on_bug_found(void)
{
	printf("A runtime bug found! stack trace:");
	gnu_libc_print_stack_trace();
}

void lxc_load_embedded_modules
(
	const char** errors,
	int maxlength
)
{
	//TODO

#ifdef LXC_EMBED_MODULE_ARITHMETIC
	lxc_load_library(&logxcontroll_loadable_library_arithmetic, errors, sizeof(errors));
#endif

#ifdef LXC_EMBED_MODULE_POSIX
	lxc_load_library(&logxcontroll_loadable_library_posix, errors, sizeof(errors));
#endif

}

#if __STDC_VERSION__ >= 201101L
#pragma message "build target: C11"
//TODO http://en.cppreference.com/w/c/thread
//TODO

#endif


#if _WIN32
#pragma message "build target: Win32"
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

int short_lock_init(short_lock* spin)
{
	return pthread_spin_init(spin, 0);
}

int short_lock_lock(short_lock* spin)
{
	return pthread_spin_lock(spin);
}

int short_lock_trylock(short_lock* spin)
{
	return pthread_spin_trylock(spin);
}

int short_lock_unlock(short_lock* spin)
{
	return pthread_spin_unlock(spin);
}

int short_lock_destroy(short_lock* spin)
{
	return pthread_spin_destroy(spin);
}



int long_lock_init(long_lock* mutex)
{
	return pthread_mutex_init(mutex, NULL);
}

int long_lock_lock(long_lock* mutex)
{
	return pthread_mutex_lock(mutex);
}

int long_lock_trylock(long_lock* mutex)
{
	return pthread_mutex_trylock(mutex);
}

int long_lock_unlock(long_lock* mutex)
{
	return pthread_mutex_unlock(mutex);
}

int long_lock_destroy(long_lock* mutex)
{
	return pthread_mutex_destroy(mutex);
}

#elif __unix__ // all unices not caught above
    // Unix
#elif defined(_POSIX_VERSION)
    // POSIX
#else
#   error "Unknown compiler"
#endif

