
#include "test-core/test_core.h"

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

