/*
 * worker_pool.c
 *
 *  Created on: 2016.03.17.
 *      Author: szupervigyor
 */

#include "core/logxcontroll.h"

static pthread_spinlock_t queue_free_spin;
struct queue_element* free_head = NULL;
struct queue_element* free_tail = NULL;


static pthread_spinlock_t queue_busy_spin;
struct queue_element* busy_head = NULL;
struct queue_element* busy_tail = NULL;

static inline struct pool_thread* align_known_rrt(struct rerunnable_thread* rrt)
{
	return (struct pool_thread*)(((char*)rrt)-(sizeof(struct queue_element)));
}

static void on_release(struct rerunnable_thread* rrt)
{
	struct pool_thread* pt = align_known_rrt(rrt);
	pthread_spin_lock(&queue_busy_spin);

	queue_pop_intermediate_element
	(
		&busy_head,
		(struct queue_element*) pt,
		&busy_tail
	);

	pthread_spin_unlock(&queue_busy_spin);

	pthread_spin_lock(&queue_free_spin);
	queue_add_element(&free_head, (struct queue_element*) pt, &free_tail);
	pthread_spin_unlock(&queue_free_spin);
}

static struct pool_thread* new_pool_thread()
{
	struct pool_thread* ret =
		(struct pool_thread*) malloc_zero(sizeof(struct pool_thread));

	rrt_init(&(ret->rerunnable));
	ret->rerunnable.on_release_callback = on_release;
	rrt_start(&(ret->rerunnable));
	return ret;
}

void lxc_submit_asyncron_task(void (*funct)(void*), void* param)
{
	pthread_spin_lock(&queue_free_spin);

	struct pool_thread* use =
		(struct pool_thread*) queue_pop_tail_element(&free_head, &free_tail);

	pthread_spin_unlock(&queue_free_spin);


	if(NULL == use)
	{
		use = new_pool_thread();
	}


	pthread_spin_lock(&queue_busy_spin);
	queue_add_element(&busy_head, &use->queue_element, &busy_tail);
	pthread_spin_unlock(&queue_busy_spin);

	rrt_try_rerun_if_free(&(use->rerunnable), funct, param);
}

void lxc_init_thread_pool()
{
	pthread_spin_init(&queue_free_spin, 0);
	pthread_spin_init(&queue_busy_spin, 0);
}

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
