/*
 * rerunnable_thread.c
 *
 *  Created on: 2016.03.17.
 *      Author: szupervigyor
 */

#include "core/logxcontroll.h"

static inline void rt_short_lock(struct rerunnable_thread* rrt)
{
	short_lock_lock(&(rrt->rt_lock));
}

static inline void rt_short_unlock(struct rerunnable_thread* rrt)
{
	short_lock_unlock(&(rrt->rt_lock));
}

static inline int atomic_get_state(struct rerunnable_thread* rrt)
{
	enum rerunnable_thread_state state = 0;
	rt_short_lock(rrt);
	state = rrt->status;
	rt_short_unlock(rrt);
	return state;
}

static inline void atomic_update_state
(
	struct rerunnable_thread* rrt,
	enum rerunnable_thread_state stat
)
{
	rt_short_lock(rrt);
	rrt->status = stat;
	rt_short_unlock(rrt);
}

static void notify_job(struct rerunnable_thread* rrt)
{
	pthread_mutex_lock(&(rrt->mutex));
	pthread_cond_broadcast(&(rrt->has_job_condition));
	pthread_mutex_unlock(&(rrt->mutex));
}

static void wait_for_job(struct rerunnable_thread* rrt)
{
	pthread_mutex_lock(&(rrt->mutex));
	if(rrt_idle == atomic_get_state(rrt))
	{
		pthread_cond_wait(&(rrt->has_job_condition), &(rrt->mutex));
	}
	pthread_mutex_unlock(&(rrt->mutex));
}

bool rrt_try_rerun_if_free
(
	struct rerunnable_thread* rrt,
	void (*function)(void*),
	void* param
)
{
	bool ret = false;
	rt_short_lock(rrt);
	if(rrt_idle == rrt->status)
	{
		ret = true;
		rrt->status = rrt_busy;

		//we setup the function and parameter
		rrt->run = function;
		rrt->parameter = param;

		//we preserved the thread, so we can unlock
		rt_short_unlock(rrt);

		//then notify the thread, it can work now
		notify_job(rrt);
	}
	else
	{
		rt_short_unlock(rrt);
	}
	return ret;
}

//shutdown request poison reference
static void pointer_on_shutdown_request(void* param)
{}

static void try_invoke_callback
(
	struct rerunnable_thread* rrt,
	enum rrt_callback_point point,
	void (*funct)(void*),
	void* param
)
{
	void (*re)
	(
		struct rerunnable_thread*,
		enum rrt_callback_point,
		void (*funct)(void*), void*
	)
		= rrt->on_release_callback;

	if(NULL != re)
	{
		re(rrt, point, funct, param);
	}
}

static void* executor_function(void* param)
{
	struct rerunnable_thread* rrt = (struct rerunnable_thread* ) param;
	pthread_detach(pthread_self());
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

		rt_short_lock(rrt);

		void (*funct)(void*) = rrt->run;
		void* param = rrt->parameter;

		if
		(
			rrt_shutdown_requested == rrt->status
			&&
			pointer_on_shutdown_request == funct
		)
		{
			//we get notified because of shutdown.
			rt_short_unlock(rrt);
			break;
		}


		rt_short_unlock(rrt);
		funct(param);

		try_invoke_callback
		(
			rrt,
			rrt_right_after_executed,
			funct,
			param
		);

		rrt->run = NULL;
		rrt->parameter = NULL;

		rt_short_lock(rrt);
		//after task done, if shutdown requested, we perform it.
		if(rrt_shutdown_requested == rrt->status)
		{
			rt_short_unlock(rrt);
			break;
		}

		rrt->status = rrt_idle;
		rt_short_unlock(rrt);

		try_invoke_callback
		(
			rrt,
			rrt_after_become_idle,
			funct,
			param
		);
	}

	atomic_update_state(rrt, rrt_exited);
	pthread_exit(NULL);
	return NULL;
}

void rrt_init(struct rerunnable_thread* rrt)
{
	memset(rrt, 0, sizeof(struct rerunnable_thread));
	pthread_mutex_init(&(rrt->mutex), NULL);
	short_lock_init(&rrt->rt_lock);
	pthread_cond_init(&(rrt->has_job_condition), NULL);

	rrt->status = rrt_initalized;
}

int rrt_start(struct rerunnable_thread* rrt)
{
	int ret = -1;
	rt_short_lock(rrt);
	if(rrt_initalized == rrt->status)
	{
		//there is a short duration where the started thread doesn't set the
		//status to rrt_idle yet, under this time we fail to submit a task
		//for this newly created thread, so i set this before thread start
		rrt->status = rrt_idle;
		ret =	pthread_create
				(
					&(rrt->thread),
					NULL,
					executor_function,
					(void*) rrt
				);
	}
	rt_short_unlock(rrt);
	return ret;
}


bool rrt_is_free(struct rerunnable_thread* rrt)
{
	enum rerunnable_thread_state state = 0;
	rt_short_lock(rrt);
	state = rrt->status;
	rt_short_unlock(rrt);
	return state == rrt_idle;
}

enum lxc_errno rrt_graceful_shutdown(struct rerunnable_thread* rrt)
{
	enum rerunnable_thread_state state;
	int i = 0;
	while(++i < 150)
	{
		rt_short_lock(rrt);
		if(!long_lock_trylock(&rrt->mutex))
		{
			rt_short_unlock(rrt);
			continue;
		}

		state = rrt->status;
		if(rrt_idle == state)
		{
			rrt->status = rrt_shutdown_requested;
			rrt->run = pointer_on_shutdown_request;
			long_lock_unlock(&rrt->mutex);
			rt_short_unlock(rrt);
			notify_job(rrt);
			return SUCCESS;
		}
		else if(rrt_busy == state)
		{
			rrt->status = rrt_shutdown_requested;
			long_lock_unlock(&rrt->mutex);
			rt_short_unlock(rrt);
			return SUCCESS;
		}
		else
		{
			long_lock_unlock(&rrt->mutex);
			rt_short_unlock(rrt);
		}

		//if(rrt_initalized == state || rrt_exited == state) //in any other case
		{
			return LXC_ERROR_ILLEGAL_REQUEST;
		}
	}

	return LXC_ERROR_TOO_HIGH_CONCURRENCY;
}

enum rerunnable_thread_state rrt_get_state(struct rerunnable_thread* rrt)
{
	return atomic_get_state(rrt);
}

int rrt_poll_wait_exit(struct rerunnable_thread* rrt)
{
	enum rerunnable_thread_state state = rrt_get_state(rrt);
	if(rrt_exited == state)
	{
		return 0;
	}

	if(rrt_shutdown_requested != state)
	{
		return LXC_ERROR_ILLEGAL_REQUEST;
	}

	do
	{
		if(rrt_get_state(rrt) == rrt_exited)
		{
			return 0;
		}
		usleep(100000); //100 ms
	}
	while(1);

	return 0;//"clean" compile
}

int rrt_destroy_thread(struct rerunnable_thread* rrt)
{
	enum rerunnable_thread_state state = rrt_get_state(rrt);
	if(rrt_exited != state && rrt_initalized != state)
	{
		return 1;
	}

	if(rrt_exited == state)
	{
		//pthread_detach(rrt->thread);
	}

	pthread_mutex_destroy(&(rrt->mutex));
	pthread_spin_destroy(&(rrt->rt_lock));
	pthread_cond_destroy(&(rrt->has_job_condition));

	return 0;
}

