/*
 * logxcontroll.c
 *
 *  Created on: 2016.02.27.
 *      Author: szupervigyor
 */


#include "core/logxcontroll.h"

static bool logxcontroll_intialized = false;

int logxcontroll_init_environment()
{
	if(logxcontroll_intialized)
	{
		return 0;
	}

	//print stack trace on memory violation and other abort cases
//	signal(SIGSEGV, print_stack_trace_then_terminalte);
//	signal(SIGABRT, print_stack_trace_then_terminalte);

	/*** initialize generic library ***/
	lxc_init_generic_library();

	//TODO lxc_init_thread_pool();


	/*** Register built in Libraries ***/
	const char* errors[20];
	memset(errors, 0, sizeof(errors));

	lxc_load_library(&logxcontroll_loadable_library_builtin, errors, 20);


	lxc_load_embedded_modules(errors, 20);

	//lxc_load_shared_library("/home/szupervigyor/projektek/LogxKontroll/WS/Liblxc_ieee1003/Default/libLiblxc_ieee1003", errors, 200);

	lxc_thread_init_env();
	TEST_ASSERT_EQUAL(0, wp_init(&LXC_SYS_DEFAULT_WORKER_POOL));

	logxcontroll_intialized = true;

	return true;
}

int logxcontroll_destroy_environment()
{
	if(!logxcontroll_intialized)
	{
		return LXC_ERROR_ILLEGAL_REQUEST;
	}

	TEST_ASSERT_EQUAL(0, wp_shutdown(&LXC_SYS_DEFAULT_WORKER_POOL));
	TEST_ASSERT_EQUAL(0, wp_wait_exit(&LXC_SYS_DEFAULT_WORKER_POOL));
	TEST_ASSERT_EQUAL(0, wp_destroy(&LXC_SYS_DEFAULT_WORKER_POOL));
	lxc_thread_destroy_env();

	logxcontroll_intialized = false;
	return 0;
}


void (*logxcontroll_after_bootstrapping)(void) = NULL;

void logxcontroll_main()
{
	logxcontroll_init_environment();

	if(NULL != logxcontroll_after_bootstrapping)
	{
		logxcontroll_after_bootstrapping();
	}

	//TODO construct bootstrapping workspace

	//returns if last thread is shutted down
	//lxc_wait_thread_pool_shutdown();
}
