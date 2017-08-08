/*
 * lxc_execution.h
 *
 *  Created on: 2016.02.15.
 *      Author: szupervigyor
 */

#ifndef LXC_EXECUTION_H_
#define LXC_EXECUTION_H_

#include "logxcontroll.h"

struct lxc_task
{
	Gate instance;
	int index;
	LxcValue value;
};

struct lxc_task;
typedef struct lxc_task* Task;

Task lxc_create_task(Gate instance, LxcValue value, int index);

void lxc_execute_task(Task task);

void lxc_do_execute(Gate g, LxcValue value, uint index);

void lxc_execute_then_release(Task t);

void lxc_execution_default_behavior(Gate instance, Signal type, int subtype, LxcValue value, uint index);

void lxc_execution_loopbreaker(Gate instance, Signal type, int subtype, LxcValue value, uint index);


struct worker_pool LXC_SYS_DEFAULT_WORKER_POOL;

int lxc_crutch_wp_exec(void(*func)(void*), void* param);


#endif /* LXC_EXECUTION_H_ */
