/*
 * lxc_execution.c
 *
 *  Created on: 2016.02.15.
 *      Author: szupervigyor
 */

#include "logxcontroll.h"

Task lxc_create_task(Gate instance, LxcValue value, int index)
{
	Task ret = malloc(sizeof(struct lxc_task));
	ret->instance = instance;
	ret->value = value;
	ret->index = index;

	//that's the only thing why i implement own Task
	//if we submit a delayed task reference maybe we forget about
	//value's reference counting (if it's used)
	//and thing get smarter with the other part:
	//lxc_execute_task automatically execute and releases
	//value if no more reference.
	lxc_reference_value(value);
	return ret;
}

void lxc_execute_task(Task task)
{
	Gate g = task->instance;
	LxcValue value = task->value;
	g->behavior->execute
	(
		g,
		NULL == value? NULL : value->type,
		task->wire->subtype,
		value,
		task->index
	);

	lxc_unreference_value(value);
}
