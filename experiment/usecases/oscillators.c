/*
 * oscillators.c
 *
 *  Created on: 2017. 07. 11.
 *      Author: Dankó Dávid
 */
#include "core/logxcontroll.h"
#include "experiment/usecases/circuit_test_utils.h"

static int RISING_EDGE_COUNT;
static LxcValue RISING_EDGE_PREVIOUS_VALUE;

void rising_edge_listener(Gate instance, Signal type, int subtype, LxcValue value, uint index)
{
	if(&lxc_signal_bool != type || 0 != subtype)
	{
		return;
	}

	if(NULL == RISING_EDGE_PREVIOUS_VALUE)
	{
		RISING_EDGE_PREVIOUS_VALUE = value;
		return;
	}

	bool prev = unsafe_extract_boolean(RISING_EDGE_PREVIOUS_VALUE);
	bool crnt = unsafe_extract_boolean(value);
	//printf("current value: %s\n", crnt ? "true":"false");
	if(crnt && !prev)
	{
		++RISING_EDGE_COUNT;
	}

	RISING_EDGE_PREVIOUS_VALUE = value;
}


/**
 * 			back	/---\    	output	   _   _   _
 * 		+-----------| A |o--+----------> _| |_| |_|  .....
 * 		|			\---/	|
 * 		|					|
 * 		|	/---\ w	/---\	|
 * 		+--o| C |--o| B |---+
 * 			\---/	\---/
 *
 *
 * */
static IOCircuit create_bool_oscillator(void)
{
	IOCircuit circ = create_basic_network_driver_sniffer_network();

	struct puppet_gate_instance* sniffer =
		(struct puppet_gate_instance*) lxc_circuit_get_gate_by_refdes(circ, "network sniffer");

	sniffer->execute = rising_edge_listener;

	Wire back = add_new_primitive_wire_to_circuit(circ, &lxc_signal_bool, "back");
	Wire output = add_new_primitive_wire_to_circuit(circ, &lxc_signal_bool, "output");
	Wire w = add_new_primitive_wire_to_circuit(circ, &lxc_signal_bool, "w");

	wiring_input(&sniffer->base.base, 0, output);
	back->current_value = lxc_bool_constant_value_false.value;

	Gate A = add_new_gate_to_circuit(circ, "bool not", "A");
	Gate B = add_new_gate_to_circuit(circ, "bool not", "B");
	Gate C = add_new_gate_to_circuit(circ, "bool not", "C");

	wiring_output(output, A, 0);
	wiring_input(B, 0, output);

	wiring_output(w, B, 0);
	wiring_input(C, 0, w);

	wiring_output(back, C, 0);
	wiring_input(A, 0, back);

	return circ;
}

static struct worker_pool worker_pool;

static void notify_gate_test(Gate g)
{
	g->execution_behavior(g, NULL, 0, NULL, 0);
}


void async_execution(Gate instance, Signal type, int subtype, LxcValue value, uint index)
{
	Task t = lxc_create_task(instance, value, index);
	int ret;
	TEST_ASSERT_EQUAL(0, ret = wp_submit_task(&worker_pool, lxc_execute_then_release, t));
	if(0 != ret)
	{
		printf("Can't submint task: %d \n", ret);
		abort();
	}
}

void bool_gate_oscillator_1_async(void)
{
	logxcontroll_init_environment();

	wp_init(&worker_pool);

	IOCircuit circ = create_bool_oscillator();
	lxc_circuit_get_gate_by_refdes(circ, "A")->execution_behavior = async_execution;

	lxc_circuit_set_all_gate_enable(circ, true);
	notify_gate_test(lxc_circuit_get_gate_by_refdes(circ, "A"));
	sleep(3);
	lxc_circuit_set_all_gate_enable(circ, false);

	printf("ring oscillator (1 async) produced %d rising edges under 3 sec\n", RISING_EDGE_COUNT);
	TEST_ASSERT_TRUE(RISING_EDGE_COUNT > 100);

	lxc_test_destroy_worker_pool(&worker_pool);

	lxc_circuit_destroy(circ);
	logxcontroll_destroy_environment();
}

void oscillator_1_async(int argc, char **argv, int start_from)
{
	bool_gate_oscillator_1_async();
}

static void test_scenario_bool_gate_oscillator_1_async(void)
{
	logxcontroll_init_environment();
	bool_gate_oscillator_1_async();
	logxcontroll_destroy_environment();
}


void bool_gate_oscillator_3_async(void)
{
	wp_init(&worker_pool);

	IOCircuit circ = create_bool_oscillator();
	lxc_circuit_get_gate_by_refdes(circ, "A")->execution_behavior = async_execution;
	lxc_circuit_get_gate_by_refdes(circ, "B")->execution_behavior = async_execution;
	lxc_circuit_get_gate_by_refdes(circ, "C")->execution_behavior = async_execution;

	lxc_circuit_set_all_gate_enable(circ, true);
	notify_gate_test(lxc_circuit_get_gate_by_refdes(circ, "A"));
	sleep(3);
	lxc_circuit_set_all_gate_enable(circ, false);


	printf("ring oscillator (3 async) produced %d rising edges under 3 sec\n", RISING_EDGE_COUNT);
	TEST_ASSERT_TRUE(RISING_EDGE_COUNT > 100);

	lxc_test_destroy_worker_pool(&worker_pool);

	lxc_circuit_destroy(circ);
}

void oscillator_3_async(int argc, char **argv, int start_from)
{
	 bool_gate_oscillator_3_async();
}


static void test_scenario_bool_gate_oscillator_3_async(void)
{
	logxcontroll_init_environment();
	bool_gate_oscillator_3_async();
	logxcontroll_destroy_environment();
}


static void task_disable_circuit_after_3_sec(IOCircuit circ)
{
	sleep(3);
	lxc_circuit_set_all_gate_enable(circ, false);
}


void bool_gate_oscillator_1_loopbreaker(void)
{
	wp_init(&worker_pool);

	IOCircuit circ = create_bool_oscillator();
	lxc_circuit_get_gate_by_refdes(circ, "A")->execution_behavior = lxc_execution_loopbreaker;

	wp_submit_task(&worker_pool, task_disable_circuit_after_3_sec, circ);
	lxc_circuit_set_all_gate_enable(circ, true);

	notify_gate_test(lxc_circuit_get_gate_by_refdes(circ, "A"));

	printf("ring oscillator (3 loopbreaker) produced %d rising edges under 3 sec\n", RISING_EDGE_COUNT);
	TEST_ASSERT_TRUE(RISING_EDGE_COUNT > 100);

	lxc_test_destroy_worker_pool(&worker_pool);

	lxc_circuit_destroy(circ);
}

void oscillator_1_loopbreaker(int argc, char **argv, int start_from)
{
	bool_gate_oscillator_1_loopbreaker();
}

static void test_scenario_bool_gate_oscillator_1_loopbreaker(void)
{
	logxcontroll_init_environment();
	bool_gate_oscillator_1_loopbreaker();
	logxcontroll_destroy_environment();
}


void bool_gate_oscillator_3_loopbreaker(void)
{
	wp_init(&worker_pool);

	IOCircuit circ = create_bool_oscillator();
	lxc_circuit_get_gate_by_refdes(circ, "A")->execution_behavior = lxc_execution_loopbreaker;
	lxc_circuit_get_gate_by_refdes(circ, "B")->execution_behavior = lxc_execution_loopbreaker;
	lxc_circuit_get_gate_by_refdes(circ, "C")->execution_behavior = lxc_execution_loopbreaker;

	wp_submit_task(&worker_pool, task_disable_circuit_after_3_sec, circ);
	lxc_circuit_set_all_gate_enable(circ, true);

	notify_gate_test(lxc_circuit_get_gate_by_refdes(circ, "A"));

	printf("ring oscillator (3 loopbreaker) produced %d rising edges under 3 sec\n", RISING_EDGE_COUNT);
	TEST_ASSERT_TRUE(RISING_EDGE_COUNT > 100);

	lxc_test_destroy_worker_pool(&worker_pool);

	lxc_circuit_destroy(circ);
	logxcontroll_destroy_environment();
}

void oscillator_3_loopbreaker(int argc, char **argv, int start_from)
{
	bool_gate_oscillator_3_loopbreaker();
}

static void test_scenario_bool_gate_oscillator_3_loopbreaker(void)
{
	logxcontroll_init_environment();
	bool_gate_oscillator_3_loopbreaker();
	logxcontroll_destroy_environment();
}

void oscillator(int argc, char **argv, int start_from)
{
	struct case_option** OPTS = NULL;
	options_register(&OPTS, "1_async", oscillator_1_async);
	options_register(&OPTS, "3_async", oscillator_3_async);

	options_register(&OPTS, "1_loopbreaker", oscillator_1_loopbreaker);
	options_register(&OPTS, "3_loopbreaker", oscillator_3_loopbreaker);

	char* ref = NULL;
	if(argc > 2)
	{
		ref = argv[2];
		int i=-1;
		while(NULL != OPTS[++i])
		{
			if(0 == strcmp(ref, OPTS[i]->name))
			{
				logxcontroll_init_environment();
				OPTS[i]->funct(argc, argv, 2);
				logxcontroll_destroy_environment();
				options_release(OPTS);
				return;
			}
		}
	}

	printf("Given oscillator: \"%s\" not found.\n", ref);
	printf("Available oscillators:\n");
	{
		int i = -1;
		while(NULL != OPTS[++i])
		{
			printf("==> \"%s\"\n", OPTS[i]->name);
		}
	}
	options_release(OPTS);
}
