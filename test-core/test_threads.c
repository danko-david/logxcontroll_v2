
#include "core/logxcontroll.h"
#include "np.h"

struct switch_holder
{
	volatile bool value;
};

void thread_set_true(struct switch_holder* sw)
{
	sw->value = true;
}

void thread_set_false(struct switch_holder* sw)
{
	sw->value = false;
}

void thread_wait_until_true(struct switch_holder* sw)
{
	while(!sw->value)
	{
		printf("waiting for condition become true\n");
		usleep(100000);
	}
}


/**
 *  Polling happens right before the first waiting, if the thread already
 * reached the state it's return 0, continues the wait with the specified
 * wait_unit_time_us (microsecounds)
 *
 * @returns numbers of polling after the thread reached the specified status
 * or -1 on timeout.
 * */
int wait_thread_for_state
(
	struct rerunnable_thread* thread,
	enum rerunnable_thread_state state,
	int wait_unit_time_us,
	int try_max_count
)
{
	int times = 0;
	do
	{
		if(rrt_get_state(thread) == state)
		{
			return times;
		}
		printf("waiting for state\n");
		usleep(wait_unit_time_us);
	}
	while(++times < try_max_count);

	return -1;
}

void print_usual_wait(const char* info, int reach)
{
	printf("thread %s after %d number of 0.1 s waiting\n", info, reach);
}

static void print_thread_state(struct rerunnable_thread* rrt, void (*func)(void), void*)
{
	enum rerunnable_thread_state state = rrt_get_state(rrt);
	printf("Thread job done, state: %d\n", state);
}

static void assert_thread_reach_state
(
	const char* info,
	struct rerunnable_thread* rrt,
	enum rerunnable_thread_state state
)
{
	int reach = wait_thread_for_state(rrt, state, 100000, 30);
	//we shouldn't exceed the maximal waiting time. 30* 100 ms.
	if(-1 == reach)
	{
		printf
		(
			"thread state isn't reached `%s`, state is now %d, desired state: %d\n",
			info,
			rrt_get_state(rrt),
			state
		);
	}
	else
	{
		print_usual_wait(info, reach);
	}
	NP_ASSERT_NOT_EQUAL(-1, reach);
}

static struct rerunnable_thread* usual_test_start()
{
	struct rerunnable_thread* thread = malloc(sizeof(struct rerunnable_thread));
	rrt_init(thread);

	thread->on_release_callback = print_thread_state;

	//must be in initalized state.
	NP_ASSERT_EQUAL(rrt_initalized, rrt_get_state(thread));

	//and may not accept task yet.
	NP_ASSERT_EQUAL
	(
		false,
		rrt_try_rerun_if_free
		(
			thread,
			(void (*)(void*)) thread_set_true,
			NULL
		)
	);


	NP_ASSERT_EQUAL(0, rrt_start(thread));


	//can't start again (after start)
	NP_ASSERT_NOT_EQUAL(0, rrt_start(thread));

	assert_thread_reach_state("started and go idle", thread, rrt_idle);
	return thread;
}

static void usual_thread_destroy(struct rerunnable_thread* thread)
{
	{
		rrt_graceful_shutdown(thread);
		assert_thread_reach_state
		(
			"thread exited",
			thread,
			rrt_exited
		);
	}

	//can't start again (after destroyed)
	NP_ASSERT_NOT_EQUAL(0, rrt_start(thread));

	//then able to destroy
	NP_ASSERT_EQUAL(0, rrt_destroy_thread(thread));

	free(thread);
}

static void test_rerunnable_thread_full_functionality(void)
{
	struct rerunnable_thread* thread = usual_test_start();
	//can't start again (after started)
	NP_ASSERT_NOT_EQUAL(0, rrt_start(thread));

	//at this point it should be free.
	NP_ASSERT_TRUE(rrt_is_free(thread));

	//with another "words"
	NP_ASSERT_EQUAL(rrt_idle, rrt_get_state(thread));

	{
		//testing for job execution, it will be really done?
		struct switch_holder sw;
		sw.value = false;

		//thread must be in idle state, so it's should accept the task.
		NP_ASSERT_EQUAL
		(
			true,
			rrt_try_rerun_if_free
			(
				thread,
				(void (*)(void*)) thread_set_true,
				&sw
			)
		);

		assert_thread_reach_state
		(
			"done the thread_set_true task and go idle",
			thread,
			rrt_idle
		);

		//the other thread set this value true.
		NP_ASSERT_EQUAL(true, sw.value);
	}

	{
		//testing for: thread may not accept another job
		struct switch_holder sw;
		sw.value = false;

		//thread is idle again, is should accept a job again, but now,
		//the thread should wait until we set the condition to true.
		NP_ASSERT_EQUAL
		(
			true,
			rrt_try_rerun_if_free
			(
				thread,
				(void (*)(void*)) thread_wait_until_true,
				&sw
			)
		);

		assert_thread_reach_state
		(
			"get busy with wait_until_true task",
			thread,
			rrt_busy
		);

		//may not accept another task.
		NP_ASSERT_EQUAL
		(
			false,
			rrt_try_rerun_if_free
			(
				thread,
				(void (*)(void*)) thread_wait_until_true,
				&sw
			)
		);

		//still must be busy (without waiting to be busy)
		NP_ASSERT_EQUAL(rrt_busy, rrt_get_state(thread));

		//no we set the switch true, so the thread "done" the task.
		sw.value = true;

		assert_thread_reach_state
		(
			"done with wait_until_true task.",
			thread,
			rrt_busy
		);
	}

	{
		//can't destroy until shutdown
		NP_ASSERT_NOT_EQUAL(0, rrt_destroy_thread(thread));
	}

	usual_thread_destroy(thread);
}

static void test_shutdown_request_beneath_running_task(void)
{
	struct rerunnable_thread* thread = usual_test_start();

	struct switch_holder sw;
	sw.value = false;

	NP_ASSERT_EQUAL
	(
		true,
		rrt_try_rerun_if_free
		(
			thread,
			(void (*)(void*)) thread_wait_until_true,
			&sw
		)
	);

	assert_thread_reach_state
	(
		"get busy with task `thread_wait_until_true`",
		thread,
		rrt_busy
	);

	rrt_graceful_shutdown(thread);

	assert_thread_reach_state
	(
		"shutdown request beneath running task",
		thread,
		rrt_shutdown_requested
	);

	sleep(1);

	//should stay in shutdown_requested state
	assert_thread_reach_state
	(
		"shutdown request after 2 sec ",
		thread,
		rrt_shutdown_requested
	);

	//let the task done
	sw.value = true;

	//destroy checking for `exited` state
	usual_thread_destroy(thread);
}
