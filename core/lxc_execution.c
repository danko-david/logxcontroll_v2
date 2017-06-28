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
	lxc_do_execute(task->instance, task->value, task->index);
}

void lxc_do_execute(Gate g, LxcValue value, uint index)
{
	if(lxc_gate_is_enabled(g))
	{
		g->behavior->execute
		(
			g,
			NULL == value? NULL : value->type,
			NULL == value? 0: value->subtype_info,
			value,
			index
		);
	}
}

void lxc_execute_then_release(Task t)
{
	lxc_execute_task(t);
	lxc_unreference_value(t->value);
	free(t);
}
