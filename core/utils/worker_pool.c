/*
 * worker_pool.c
 *
 *  Created on: 2016.03.17.
 *      Author: szupervigyor
 */

#include "core/logxcontroll.h"

static void on_release
(
	struct rerunnable_thread* rrt,
	void (*func)(void*),
	void* param
)
{
	//UNUSED(rrt);
	//UNUSED(func);
	struct pool_thread* task  = (struct pool_thread*) param;
	struct worker_pool* pool = task->pool;
	pthread_spin_lock(&(pool->queue_busy_spin));

	queue_pop_intermediate_element
	(
		&pool->busy_head,
		&task->elem,
		&pool->busy_tail
	);

	pthread_spin_unlock(&pool->queue_busy_spin);

	pthread_spin_lock(&pool->queue_free_spin);
	queue_add_element(&pool->free_head, &task->elem, &pool->free_tail);
	pthread_spin_unlock(&pool->queue_free_spin);
}

static struct pool_thread* new_pool_thread()
{
	struct pool_thread* ret =
		(struct pool_thread*) malloc_zero(sizeof(struct pool_thread));

	rrt_init(&(ret->thread));
	ret->thread.on_release_callback = on_release;
	if(0 != rrt_start(&(ret->thread)))
	{
		return NULL;
	}

	return ret;
}

static void worker_pool_exec_function(struct pool_thread* task)
{
	task->executor(task->param);
}
//__attribute__((warn_unused_result));
int wp_submit_task(struct worker_pool* wp, void (*func)(void*), void* param)
{
	pthread_spin_lock(&wp->queue_free_spin);

	struct pool_thread* use =
		(struct pool_thread*) queue_pop_tail_element(&wp->free_head, &wp->free_tail);

	pthread_spin_unlock(&wp->queue_free_spin);

	if(NULL == use)
	{
		use = new_pool_thread();
		return LXC_ERROR_RESOURCE_BUSY;
	}

	pthread_spin_lock(&wp->queue_busy_spin);
	queue_add_element(&wp->busy_head, &use->elem, &wp->busy_tail);
	pthread_spin_unlock(&wp->queue_busy_spin);

	use->executor = func;
	use->param = param;
	//it's must be free
	rrt_try_rerun_if_free
	(
		&(use->thread),
		(void (*)(void*)) worker_pool_exec_function,
		(void*) use
	);
}

void wp_get_status(struct worker_pool* wp, int* busy, int* idle)
{


}

void wp_init(struct worker_pool* pool)
{
	memset(pool, 0, sizeof(struct worker_pool));
	pthread_spin_init(&pool->queue_free_spin, 0);
	pthread_spin_init(&pool->queue_busy_spin, 0);
}

//TODO shutdown

void lxc_wait_thread_pool_shutdown()
{
/*
	TODO later we drain all free thread and shutdown...

	TODO use a global bool logxcontroll_under_shudown
	to indicate, no more job may added to the pool.

	TODO wait all the busy thread to complete it's task.

	TODO hmm we will need some callback function for the components (gates)
	to can be do the right operation to clean shutdown the system.

	or maybe before shutdown, all gate must be disabled, so that can be the
	signal also for system shutdown, this solution results more consistent
	behavior implementation for gates... Wow that's the way what i'm looking
	for. :D

	struct pool_thread* wait;
	pthread_spin_lock(&queue_busy_spin);
	//queue_add_element(&busy_head, use, &busy_tail);
	pthread_spin_unlock(&queue_busy_spin);
*/
}
