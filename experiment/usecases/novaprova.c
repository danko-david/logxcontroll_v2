/*
 * novaprova.c
 *
 *  Created on: 2017.07.05.
 *      Author: Dankó Dávid
 *
 * source: http://novaprova.org/doc-1.0/get-start/index.html
 * source: novaprova/main.c
 */

#include "core/logxcontroll.h"

void novaprova(int argc, char **argv, int start_from)
{
    int ec = 0;
    np_runner_t *runner;

    /* Initialise the NovaProva library */
    runner = np_init();

	np_plan_t *plan = NULL;
	if(argc > start_from)
	{
		plan = np_plan_new();
		np_plan_add_specs(plan, argc-start_from, (const char **)argv+start_from);
	}
	/* Run all the discovered tests */
	ec = np_run_tests(runner, plan);

	if (plan)
	{
		np_plan_delete(plan);
	}

    /* Shut down the NovaProva library */
    np_done(runner);

    exit(ec);
}
