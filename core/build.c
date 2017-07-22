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


#elif __unix__ // all unices not caught above
    // Unix
#elif defined(_POSIX_VERSION)
    // POSIX
#else
#   error "Unknown compiler"
#endif



//#ifdef __linux__
#ifdef USE_GREEN_THREAD
#ifdef USE_VIREO
int lxc_start_new_thread
(
	thread_handle t,
	void (*executor)(void*),
	void* param
)
{
	return vireo_pcreate((vireo_thread*) t, executor, param);
}

int lxc_thread_setup_after_start()
{
	return 0;
}

int lxc_thread_init_env(void)
{
	return vireo_env_init();
}

int lxc_thread_destroy_env(void)
{
	return vireo_env_destroy();
}
#else

int lxc_start_new_thread
(
	thread_handle* t,
	void (*executor)(void*),
	void* param
)
{
	pth_attr_t attr = pth_attr_new();
	pth_attr_set(attr, PTH_ATTR_JOINABLE, FALSE);
	pth_attr_set(attr, PTH_ATTR_STACK_SIZE, 64*1024);
	pth_t ret = pth_spawn(attr, (void* (*)(void *))executor, param);
	pth_attr_destroy(attr);
	if(NULL != t)
	{
		*t = ret;
	}
	return NULL == ret?ENOMEM:0;
}

int lxc_thread_setup_after_start()
{
	return 0;
}

int lxc_thread_init_env(void)
{
	return pth_init();
}

int lxc_thread_destroy_env(void)
{
	//printf("pth_kill called\n");
	//this call might cause closing some file descriptor that
	//it doesn't have to do. return pth_kill();
	return 0;
}


#include "pth.h"

static int wrap_ret(int ret)
{
	if(TRUE == ret)
	{
		return 0;
	}
	else if(FALSE == ret)
	{
		return -1;
	}

	return ret;
}

int short_lock_init(short_lock* spin)
{
	return wrap_ret(pth_mutex_init(spin));
}

int short_lock_lock(short_lock* spin)
{
	return wrap_ret(pth_mutex_acquire(spin, 0, NULL));
}

static int apth_mutex_trylock(pth_mutex_t* lock)
{
	int ret = pth_mutex_acquire(lock, TRUE, NULL);
	if(FALSE == ret)
	{
		return EBUSY;
	}
	else
	{
		if(TRUE == ret)
		{
			//if sucessfully acquired.
			//recursice locking supported by this library but this is not an
			//expected feature.
			if(lock->mx_count > 1)
			{
				pth_mutex_release(lock);
				return EBUSY;
			}
			else
			{
				return 0;
			}
		}
		else
		{
			///another status code
			return ret;
		}
	}
}

int short_lock_trylock(short_lock* spin)
{

	return apth_mutex_trylock(spin);
}

int short_lock_unlock(short_lock* spin)
{
	return wrap_ret(pth_mutex_release(spin));
}

int short_lock_destroy(short_lock* spin)
{
	return 0;//pth_mutex_destroy(spin);
}



int long_lock_init(long_lock* mutex)
{
	return wrap_ret(pth_mutex_init(mutex));
}

int long_lock_lock(long_lock* mutex)
{
	return wrap_ret(pth_mutex_acquire(mutex, 0, NULL));
}

int long_lock_trylock(long_lock* mutex)
{
	return apth_mutex_trylock(mutex);
}

int long_lock_unlock(long_lock* mutex)
{
	return wrap_ret(pth_mutex_release(mutex));
}

int long_lock_destroy(long_lock* mutex)
{
	return 0;//pth_mutex_destroy(mutex);
}



int lxc_thread_cond_wait_init(struct conditional_wait* cw)
{
	pth_mutex_init(&cw->mutex);
	pth_cond_init(&cw->condition);
	return 0;
}

int lxc_thread_cond_wait_destroy(struct conditional_wait* cw)
{
	return 0;//pth_mutex_destroy(&cw->mutex);
	//pth_cond_destroy(&cw->condition);
}

void lxc_thread_cond_wait_notify(struct conditional_wait* cw)
{
	pth_mutex_acquire(&cw->mutex, 0, NULL);
	pth_cond_notify(&cw->condition, TRUE);
	pth_mutex_release(&cw->mutex);
}

void lxc_thread_cond_wait_wait(struct conditional_wait* cw)
{
	pth_cond_await(&cw->condition, &cw->mutex, NULL);
}

int lxc_thread_cond_wait_lock(struct conditional_wait* cw)

{
	return wrap_ret(pth_mutex_acquire(&cw->mutex, 0, NULL));
}

int lxc_thread_cond_wait_unlock(struct conditional_wait* cw)
{
	return wrap_ret(pth_mutex_release(&cw->mutex));
}

int lxc_thread_cond_wait_trylock(struct conditional_wait* cw)
{
	return wrap_ret(pth_mutex_acquire(&cw->mutex, TRUE, NULL));
}

int c_sleep(int sec)
{
	return pth_sleep(sec);
}

int c_usleep(__useconds_t us)
{
	return pth_usleep(us);
}

#endif

#else

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


int lxc_start_new_thread
(
	thread_handle t,
	void (*executor)(void*),
	void* param
)
{
	return pthread_create
	(
		t,
		NULL,
		(void *(*) (void *)) executor,
		param
	);
}

int lxc_thread_setup_after_start()
{
	pthread_detach(pthread_self());
	return 0;
}

int lxc_thread_init_env(void)
{
	return 0;
}

int lxc_thread_destroy_env(void)
{
	return 0;
}

int lxc_thread_cond_wait_init(struct conditional_wait* cw)
{
	pthread_mutex_init(&cw->mutex, NULL);
	pthread_cond_init(&cw->condition, NULL);
	return 0;
}

int lxc_thread_cond_wait_destroy(struct conditional_wait* cw)
{
	pthread_mutex_destroy(&cw->mutex);
	pthread_cond_destroy(&cw->condition);
	return 0;
}

void lxc_thread_cond_wait_notify(struct conditional_wait* cw)
{
	pthread_mutex_lock(&cw->mutex);
	pthread_cond_broadcast(&cw->condition);
	pthread_mutex_unlock(&cw->mutex);
}

void lxc_thread_cond_wait_wait(struct conditional_wait* cw)
{
	pthread_cond_wait(&cw->condition, &cw->mutex);
}

int lxc_thread_cond_wait_lock(struct conditional_wait* cw)
{
	return pthread_mutex_lock(&cw->mutex);
}

int lxc_thread_cond_wait_unlock(struct conditional_wait* cw)
{
	return pthread_mutex_unlock(&cw->mutex);
}

int lxc_thread_cond_wait_trylock(struct conditional_wait* cw)
{
	return pthread_mutex_trylock(&cw->mutex);
}

int c_sleep(int sec)
{
	return sleep(sec);
}

int c_usleep(__useconds_t us)
{
	return usleep(us);
}

#endif
