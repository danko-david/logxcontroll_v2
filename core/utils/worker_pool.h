/*
 * worker_pool.h
 *
 *  Created on: 2016.03.17.
 *      Author: szupervigyor
 */

#ifndef WORKER_POOL_H_
#define WORKER_POOL_H_

#include "core/logxcontroll.h"


struct worker_pool
{
	pthread_spinlock_t queue_free_spin;
	struct queue_element* free_head;
	struct queue_element* free_tail;


	pthread_spinlock_t queue_busy_spin;
	struct queue_element* busy_head;
	struct queue_element* busy_tail;
};

struct pool_thread
{
	struct queue_element elem;
	struct rerunnable_thread thread;
	struct worker_pool* pool;
	void (*executor)(void*);
	void* param;
};

/*
void lxc_init_thread_pool();

void lxc_wait_thread_pool_shutdown();

void lxc_submit_asyncron_task(void (*funct)(void*), void* param);

struct pool_thread
{
	struct queue_element queue_element;
	struct rerunnable_thread rerunnable;
};
*/

#endif /* WORKER_POOL_H_ */
