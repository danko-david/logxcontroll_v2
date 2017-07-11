/*
 * lxc_execution.c
 *
 *  Created on: 2016.02.15.
 *      Author: szupervigyor
 */

#include "core/logxcontroll.h"

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

struct queue
{
	struct queue_element* head;
	struct queue_element* tail;
};

struct qe_task
{
	struct queue_element qe;
	Gate instance;
	LxcValue value;
	uint index;
};

static struct queue* LOOP_BREAKER = NULL;

static struct qe_task* create_qe_task(Gate instance, LxcValue val, uint index)
{
	struct qe_task* ret = malloc_zero(sizeof(struct qe_task));
	ret->instance = instance;
	lxc_reference_value(val);
	ret->value = val;
	ret->index = index;
	return ret;
}

static void qe_task_execute_then_destroy(struct qe_task* e)
{
	lxc_do_execute(e->instance, e->value, e->index);
	lxc_unreference_value(e->value);
	free(e);
}

void lxc_execution_loopbreaker(Gate instance, Signal type, int subtype, LxcValue value, uint index)
{
	struct qe_task* task = create_qe_task(instance, value, index);
	if(NULL == LOOP_BREAKER)
	{
		LOOP_BREAKER = malloc_zero(sizeof(struct queue));

		struct qe_task* crnt = task;

		while(NULL != crnt)
		{
			qe_task_execute_then_destroy(crnt);
			crnt = (struct qe_task*) queue_pop_head_element(&LOOP_BREAKER->head, &LOOP_BREAKER->tail);
		}

		//done, wipe
		free(LOOP_BREAKER);
		LOOP_BREAKER = NULL;
	}
	else
	{
		queue_add_element
		(
			&LOOP_BREAKER->head,
			&task->qe,
			&LOOP_BREAKER->tail
		);
		return;
	}
}

