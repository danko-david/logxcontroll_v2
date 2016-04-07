/*
 * logxcontroll.c
 *
 *  Created on: 2016.02.27.
 *      Author: szupervigyor
 */


#include "core/logxcontroll.h"

static bool logxcontroll_intialized = false;

void logxcontroll_init_environment()
{
	if(logxcontroll_intialized)
		return;
	/*** initialize generic library ***/
	lxc_init_generic_library();

	lxc_init_thread_pool();


	/*** Register built in Libraries ***/
	char* errors[20];
	lxc_load_library(&logxcontroll_loadable_library_builtin, errors, sizeof(errors));
	lxc_load_library(&logxcontroll_loadable_library_bool, errors, sizeof(errors));

	lxc_load_shared_library("/home/szupervigyor/projektek/LogxKontroll/WS/Liblxc_ieee1003/Default/libLiblxc_ieee1003", errors, 200);


	logxcontroll_intialized = true;
}

void (*logxcontroll_after_bootstrapping)(void) = NULL;

void logxcontroll_main()
{
	logxcontroll_init_environment();

	if(NULL != logxcontroll_after_bootstrapping)
		logxcontroll_after_bootstrapping();

	//TODO construct bootstrapping workspace

	//returns if last thread is shutted down
	lxc_wait_thread_pool_shutdown();
}
