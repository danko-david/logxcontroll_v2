/*
 * logxcontroll.c
 *
 *  Created on: 2017.04.19.
 *      Author: szupervigyor
 */


#ifndef TEST_CORE_H_
#define TEST_CORE_H_

#include "core/logxcontroll.h"
#include "np.h"

struct switch_holder
{
	volatile bool value;
};

void thread_set_true(struct switch_holder*);
void thread_set_false(struct switch_holder*);

void thread_wait_until_true(struct switch_holder*);

int wait_thread_for_state
(
	struct rerunnable_thread* thread,
	enum rerunnable_thread_state state,
	int wait_unit_time_us,
	int try_max_count
);

int wait_switch_for_state
(
	struct switch_holder* sw,
	bool state,
	int wait_unit_time_us,
	int try_max_count
);

void assert_thread_reach_state
(
	const char* info,
	struct rerunnable_thread* rrt,
	enum rerunnable_thread_state state
);

void assert_switch_reach_state
(
	const char* info,
	struct switch_holder* sw,
	bool state
);

struct worker_pool* lxc_test_create_worker_pool()
{
	struct worker_pool* pool = malloc_zero(sizeof(struct worker_pool));
	wp_init(pool);
	return pool;
}

void lxc_test_destroy_worker_pool(struct worker_pool* wp)
{
	NP_ASSERT_EQUAL(0, wp_shutdown(wp));
	NP_ASSERT_EQUAL(0, wp_wait_exit(wp));
	NP_ASSERT_EQUAL(0, wp_destroy(wp));
}

#endif /* TEST_CORE_H_ */
