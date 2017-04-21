
#include "test-core/test_core.h"

void test_worker_pool(void)
{
	struct worker_pool wp;
	wp_init(&wp);

	struct switch_holder sw;
	sw.value = false;

	wp_submit_task(&wp, thread_set_true, &sw);

	{
		int busy;
		int idle;
	//	wp_get_status(wp, &busy, &idle);
	}
	assert_switch_reach_state
	(
		"pool set value true",
		&sw,
		true
	);
	//TODO if task running: 1 idle, 0 busy

	//TODO test for leakage

	//TODO test ping-pong task submitting

	//TODO wait shutdown
	//TODO destroy
}

//TODO shrink/spawn

