
#include "test-core/test_core.h"

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


void test_worker_pool(void)
{
	struct worker_pool* wp = lxc_test_create_worker_pool();
	struct switch_holder sw;
	sw.value = false;

	NP_ASSERT_EQUAL(0, wp_submit_task(&wp, thread_set_true, &sw));

	assert_switch_reach_state
	(
		"pool set value true",
		&sw,
		true
	);


	lxc_test_destroy_worker_pool(wp);
	//can


	//TODO if task running: 1 idle, 0 busy

	//TODO test for leakage

	//TODO test ping-pong task submitting

	//TODO wait shutdown
	//TODO destroy
}

//TODO shrink/spawn

