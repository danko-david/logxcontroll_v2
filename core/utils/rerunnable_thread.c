/*
 * rerunnable_thread.c
 *
 *  Created on: 2016.03.17.
 *      Author: szupervigyor
 */

#include "core/logxcontroll.h"

static inline void short_lock(struct rerunnable_thread* rrt)
{
	pthread_spin_lock(&(rrt->short_lock));
}

static inline void short_unlock(struct rerunnable_thread* rrt)
{
	pthread_spin_unlock(&(rrt->short_lock));
}

static inline int atomic_get_state(struct rerunnable_thread* rrt)
{
	enum rerunnable_thread_state state = 0;
	short_lock(rrt);
	state = rrt->status;
	short_unlock(rrt);
	return state;
}

static inline void atomic_update_state
(
	struct rerunnable_thread* rrt,
	enum rerunnable_thread_state stat
)
{
	short_lock(rrt);
	rrt->status = stat;
	short_unlock(rrt);
}

static void notify_job(struct rerunnable_thread* rrt)
{
	pthread_mutex_lock(&(rrt->mutex));
	pthread_cond_broadcast(&(rrt->has_job_condition));
	pthread_mutex_unlock(&(rrt->mutex));
}

static void wait_for_job(struct rerunnable_thread* rrt)
{
	if(rrt_idle != atomic_get_state(rrt))
		return;

	pthread_mutex_lock(&(rrt->mutex));
	pthread_cond_wait(&(rrt->has_job_condition), &(rrt->mutex));
	pthread_mutex_unlock(&(rrt->mutex));
}

bool rrt_try_rerun_if_free(struct rerunnable_thread* rrt, void (*function)(void*), void* param)
{
	bool ret = false;
	short_lock(rrt);
	if(rrt_idle == rrt->status)
	{
		ret = true;
		rrt->status = rrt_busy;

		//we preserved the thread, so we can unlock
		short_unlock(rrt);

		//we setup the function and parameter
		rrt->run = function;
		rrt->parameter = param;

		//then notify the thread, it can work now
		notify_job(rrt);
	}
	else
	{
		short_unlock(rrt);
	}
	return ret;
}

static void executor_function(struct rerunnable_thread* rrt)
{
	while(true)
	{
		wait_for_job(rrt);
		/**
		 * At this point, we can safety read values
		 * because the requester thread atomically
		 * updated the thread's status, so nobody will
		 * disturb us and values are also updated before
		 * we are notified.
		 */

		if(rrt_shutdown_requested == atomic_get_state(rrt))
		{
			atomic_update_state(rrt, rrt_exited);
			notify_job(rrt);
			return;
		}
		else
		{
			rrt->run((void*)rrt->parameter);
		}

		//after task done, if shutdown requested, we perform it.
		if(rrt_shutdown_requested == atomic_get_state(rrt))
		{
			atomic_update_state(rrt, rrt_exited);
			notify_job(rrt);
			return;
		}

		atomic_update_state(rrt, rrt_idle);
		void (*volatile re)(struct rerunnable_thread*) = rrt->on_release_callback;
		if(NULL != re)
		{
			re(rrt);
		}
	}
}

void rrt_init_rerunnable_thread(struct rerunnable_thread* rrt)
{
	memset(rrt, 0, sizeof(struct rerunnable_thread));
	pthread_mutex_init(&(rrt->mutex), NULL);
	pthread_spin_init(&(rrt->short_lock), 0);
	pthread_cond_init(&(rrt->has_job_condition), NULL);

	rrt->status = rrt_initalized;
}

bool rrt_start(struct rerunnable_thread* rrt)
{
	bool ret = false;
	short_lock(rrt);
	if(rrt_initalized == rrt->status)
	{
		//there is a short duration where the started thread doesn't set the
		//status to rrt_idle yet, under this time we fail to submit a task
		//for this newly created thread, so i set this before thread start
		rrt->status = rrt_idle;
		int ex =	pthread_create
					(
						&(rrt->thread),
						NULL,
						(void *(*) (void *)) executor_function,
						(void*) rrt
					);

		//pthread_detach(&(rrt->thread));

		ret = (ex == 0);
	}
	short_unlock(rrt);
	return ret;
}


bool rrt_is_free(struct rerunnable_thread* rrt)
{
	enum rerunnable_thread_state state = 0;
	short_lock(rrt);
	state = rrt->status;
	short_unlock(rrt);
	return state == rrt_idle;
}

void rrt_graceful_shutdown(struct rerunnable_thread* rrt)
{
	short_lock(rrt);
	if(rrt_idle == rrt->status)
	{
		rrt->status = rrt_shutdown_requested;
		short_unlock(rrt);
		notify_job(rrt);
	}
	else if(rrt_busy == rrt->status)
	{
		rrt->status = rrt_shutdown_requested;
		short_unlock(rrt);
	}
}

enum rerunnable_thread_state rrt_get_state(struct rerunnable_thread* rrt)
{
	return atomic_get_state(rrt);
}

void rrt_wait_exit(struct rerunnable_thread* rrt)
{
	void* ret = NULL;
	pthread_join(rrt->thread, &ret);
	/*
	short_lock(rrt);
	if(rrt_exited == rrt->status)
	{
		short_unlock(rrt);
		return;
	}
	else
	{
		pthread_mutex_lock(&(rrt->mutex));
		short_unlock(rrt);

		pthread_cond_wait(&(rrt->has_job_condition), &(rrt->mutex));
		pthread_mutex_unlock(&(rrt->mutex));
	}
	*/
}

