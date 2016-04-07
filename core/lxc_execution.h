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
	Wire wire;
	LxcValue value;
	int index;
};

struct lxc_task;
typedef struct lxc_task* Task;

Task lxc_create_task(Gate instance, LxcValue value, int index);

void lxc_execute_task(Task task);

void default_execution_behavior(Gate instance, Signal type, LxcValue value, uint index);



#endif /* LXC_EXECUTION_H_ */
