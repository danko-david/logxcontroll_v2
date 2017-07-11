/*
 * test_complex_op.c
 *
 *  Created on: 2017.05.18.
 *      Author: szupervigyor
 *
 * Some test to describe functionality.
 */
#ifdef ITS_ALWAYS_FALSE
//#ifdef INCLUDE_NOVAPROVA

#include "test-core/test_core.h"

/************************ PREVIOUS implentetion area **************************/

Wire lxc_create_primitive_wire(Signal signal)
{
	return lxc_wire_create(signal);
}

typedef int refcount_t;

struct obj_str_arr_pnt
{
	void* rtt_type;
	refcount_t refcount;
	char** array;
};

static int wire_iterator_add_ref_des(Wire w, void*** arr)
{
	array_pnt_append_element(arr, w->ref_des);
}

struct obj_str_arr_pnt*  lxc_circuit_get_all_wire_refdes(IOCircuit circ)
{
	struct obj_str_arr_pnt* ret = malloc_zero(sizeof(struct obj_str_arr_pnt));
	ret->refcount = 1;
	array_pnt_init((void***) &ret->array);

	hashmap_iterate(circ->wires, (PFany) wire_iterator_add_ref_des, &ret->array);

	return ret;
}

bool lxc_check_gate_exists(const char* name)
{
	return NULL != get_gate_entry_by_name(name);
}







/**
 *
 * 				/---\  a_c	/---\  c_d	/---\
 * input ---+---| A	|o------| C	|o------|xor|o--- output
 * 			|	\---/		\---/	+---| D	|
 * 			|						|	\---/
 * 			|	/---\	b_d			|
 * 			+---| B	|o--------------+
 * 				\---/
 *
 * */
static IOCircuit create_multistage_bool_network()
{
	TEST_ASSERT_EQUAL(true, lxc_check_gate_exists("bool not"));
	TEST_ASSERT_EQUAL(true, lxc_check_gate_exists("bool xor"));

	IOCircuit sub = create_basic_network_driver_sniffer_network();
	lxc_circuit_set_name(sub, "multistage_bool_network");

	Wire input = add_new_primitive_wire_to_circuit(sub, &lxc_signal_bool, "input");
	Wire output = add_new_primitive_wire_to_circuit(sub, &lxc_signal_bool, "output");

	Wire a_c = add_new_primitive_wire_to_circuit(sub, &lxc_signal_bool, "a_c");
	Wire c_d = add_new_primitive_wire_to_circuit(sub, &lxc_signal_bool, "c_d");
	Wire b_d = add_new_primitive_wire_to_circuit(sub, &lxc_signal_bool, "b_d");

	Gate A = add_new_gate_to_circuit(sub, "bool not", "A");
	Gate B = add_new_gate_to_circuit(sub, "bool not", "B");
	Gate C = add_new_gate_to_circuit(sub, "bool not", "C");
	Gate D = add_new_gate_to_circuit(sub, "bool xor", "D");

	//wiring up

	//input to a, b
	wiring_input(A, 0, input);
	wiring_input(B, 0, input);

	//a_c
	wiring_output(a_c, A, 0);
	wiring_input(C, 0, a_c);

	//c_d
	wiring_output(c_d, C, 0);
	wiring_input(D, 0, c_d);

	//b_d
	wiring_output(b_d, B, 0);
	wiring_input(D, 1, b_d);

	//output
	wiring_output(output, D, 0);

	Gate driver = lxc_circuit_get_gate_by_refdes(sub, "network driver");
	Gate sniffer = lxc_circuit_get_gate_by_refdes(sub, "network sniffer");

	wiring_output(input, driver, 0);
	wiring_input(sniffer, 0, output);

	return sub;
}

static void print_bool_value(Gate instance, Signal type, int subtype, LxcValue value, uint index)
{
	void* data = lxc_get_value(value);
	if(&lxc_signal_bool == type && NULL != data)
	{
		printf("Gate(%p) input %d. boolean value: %s\n", instance, index, *((bool*)data) == true?"true":"false");
	}
}

static IOCircuit generic_test_bool_unit_assemble_circuit(const char* name, bool use_secound_input)
{
	TEST_ASSERT_EQUAL(true, lxc_check_gate_exists(name));

	IOCircuit sub = lxc_circuit_create();
	lxc_circuit_set_name(sub, "single gate testbench");

	Wire I1 = add_new_primitive_wire_to_circuit(sub, &lxc_signal_bool, "I1");
	Wire O = add_new_primitive_wire_to_circuit(sub, &lxc_signal_bool, "O");
	Gate G = add_new_gate_to_circuit(sub, name, name);
	//wiring up

	wiring_input(G, 0, I1);

	if(use_secound_input)
	{
		Wire I2 = add_new_primitive_wire_to_circuit(sub, &lxc_signal_bool, "I2");
		wiring_input(G, 1, I2);
	}
	//output
	wiring_output(O, G, 0);
	//TODO wire up gates but another TODO for types
	return sub;
}

struct truth_table_entry
{
	bool I1;
	bool I2;
	bool output;
};

static void set_userdata(Gate instance, Signal type, int subtype, LxcValue value, uint index)
{
	void* data = lxc_get_value(value);
	if(&lxc_signal_bool == type && NULL != data)
	{
		struct puppet_gate_instance* gate = (struct puppet_gate_instance*) instance;
		//here we use constant value. (LxcValue true/false)
		gate->user_data = lxc_get_value(value);
	}
}

static void generic_validate_truth_table(const char* name, bool use_secound_input, struct truth_table_entry** truth_table)
{
	logxcontroll_init_environment();
	IOCircuit circ = generic_test_bool_unit_assemble_circuit(name, use_secound_input);

	Wire I1 = lxc_circuit_get_wire_by_refdes(circ, "I1");
	Wire I2 = lxc_circuit_get_wire_by_refdes(circ, "I2");

	struct puppet_gate_instance* driver = create_puppet_gate();
	{
		lxc_gate_set_refdes(&driver->base.base, "network driver");
		lxc_port_unchecked_add_new_port
		(
			&driver->base.output_ports,
			"O1",
			&lxc_signal_bool,
			0,
			NULL
		);

		lxc_port_unchecked_add_new_port
		(
			&driver->base.output_ports,
			"O2",
			&lxc_signal_bool,
			0,
			NULL
		);

		TEST_ASSERT_EQUAL(0, lxc_circuit_add_gate(circ, &driver->base.base));
		wiring_output(I1, &driver->base.base, 0);
		if(use_secound_input)
		{
			wiring_output(I2, &driver->base.base, 1);
		}
	}

	struct puppet_gate_instance* receiver = create_puppet_gate();
	{
		lxc_gate_set_refdes(&receiver->base.base, "network sniffer");
		receiver->execute = set_userdata;
		lxc_port_unchecked_add_new_port
		(
			&receiver->base.input_ports,
			"input",
			&lxc_signal_bool,
			0,
			NULL
		);

		Wire output = lxc_circuit_get_wire_by_refdes(circ, "O");
		wiring_input(&receiver->base.base, 0, output);

		TEST_ASSERT_EQUAL(0, lxc_circuit_add_gate(circ, &receiver->base.base));
	}

	lxc_circuit_set_all_gate_enable(circ, true);

	int i=0;
	while(NULL != truth_table[i])
	{
		struct truth_table_entry* ent = truth_table[i];
		lxc_drive_wire_value
		(
			&driver->base.base,
			0,
			I1,
			ent->I1?
				lxc_bool_constant_value_true.value
			:
				lxc_bool_constant_value_false.value
		);

		if(use_secound_input)
		{
			lxc_drive_wire_value
			(
				&driver->base.base,
				0,
				I2,
				ent->I2?
					lxc_bool_constant_value_true.value
				:
					lxc_bool_constant_value_false.value
			);
		}

		TEST_ASSERT_NOT_NULL(receiver->user_data);
		bool* ret = (bool*) receiver->user_data;
		TEST_ASSERT_EQUAL(ent->output, *ret);

		++i;
	}

	lxc_circuit_destroy(circ);
}

static void test_gate_not(void)
{
	struct truth_table_entry* truth[3];

	struct truth_table_entry a;
	a.I1 = true;
	a.I2 = NULL;
	a.output = false;

	struct truth_table_entry b;
	b.I1 = false;
	b.I2 = NULL;
	b.output = true;

	truth[0] = &a;
	truth[1] = &b;
	truth[2] = NULL;

	generic_validate_truth_table("bool not", false, truth);
}

static void test_gate_xor(void)
{
	struct truth_table_entry* truth[5];

	struct truth_table_entry a;
	a.I1 = true;
	a.I2 = true;
	a.output = false;
	truth[0] = &a;


	struct truth_table_entry b;
	b.I1 = false;
	b.I2 = false;
	b.output = false;
	truth[1] = &b;

	struct truth_table_entry c;
	c.I1 = false;
	c.I2 = true;
	c.output = true;
	truth[2] = &c;

	struct truth_table_entry d;
	d.I1 = true;
	d.I2 = false;
	d.output = true;
	truth[3] = &d;

	truth[4] = NULL;

	generic_validate_truth_table("bool xor", true, truth);
}


static void assert_prelling_then_release_array(void*** array)
{
	LxcValue* vs = (LxcValue*) *array;

	TEST_ASSERT_EQUAL(2, array_pnt_population(*array));
	TEST_ASSERT_EQUAL(false, unsafe_extract_boolean(vs[0]));
	TEST_ASSERT_EQUAL(true, unsafe_extract_boolean(vs[1]));
	free(*array);
	*array = NULL;
}

/**
 * if wires don't uses tokens, just propagates the values and don't removes
 * even after the gate feedbacks the "token absorption"
 *
 * this boolean network always produce true on the output
 * I := input
 * O := output
 *
 * O = !!I != !I
 * O = I != !I
 * true != !true => true != false => true
 * false != !false => true
 *
 * when value propagated, for first time the output will produce the desired
 * value.
 *
 * for the next input, the output will waggle between the true and false but
 * finally reach the true state and the circuit becomes idle.
 */
static void test_scenario_bool_gate_circuit__with_hazard(void)
{
	logxcontroll_init_environment();
	IOCircuit circ = create_multistage_bool_network();

	struct puppet_gate_instance* driver = (struct puppet_gate_instance*)
		lxc_circuit_get_gate_by_refdes(circ, "network driver");

	struct puppet_gate_instance* receiver = (struct puppet_gate_instance*)
			lxc_circuit_get_gate_by_refdes(circ, "network sniffer");

	Wire input = lxc_circuit_get_wire_by_refdes(circ, "input");

	lxc_circuit_set_all_gate_enable(circ, true);

	Gate drv = &driver->base.base;

	void*** result_array = (void***) &receiver->user_data;

	{
		array_pnt_init(result_array);
		lxc_drive_wire_value(drv, 0, input, lxc_bool_constant_value_false.value);
		//after first write: only one true value produced, because previously no
		//value assigned to the wire for first time.
		TEST_ASSERT_EQUAL(1, array_pnt_population((void**)receiver->user_data));
		TEST_ASSERT_EQUAL(true, unsafe_extract_boolean(((LxcValue*)receiver->user_data)[0]));
		free(*result_array);
		*result_array = NULL;
	}

	{
		array_pnt_init(result_array);
		lxc_drive_wire_value(drv, 0, input, lxc_bool_constant_value_true.value);
		//next time, because values already assigned to the wires, prelling occurs
		assert_prelling_then_release_array(result_array);
	}

	{
		array_pnt_init(result_array);
		lxc_drive_wire_value(drv, 0, input, lxc_bool_constant_value_false.value);
		assert_prelling_then_release_array(result_array);
	}

	{
		array_pnt_init(result_array);
		lxc_drive_wire_value(drv, 0, input, lxc_bool_constant_value_true.value);
		assert_prelling_then_release_array(result_array);
	}

	lxc_circuit_destroy(circ);

	logxcontroll_destroy_environment();
}

static void assert_not_prelling_then_release_array(void*** array)
{
	LxcValue* vs = (LxcValue*) *array;

	TEST_ASSERT_EQUAL(1, array_pnt_population(*array));
	TEST_ASSERT_EQUAL(true, unsafe_extract_boolean(vs[0]));
	free(*array);
	*array = NULL;
}

static void wire_nonhazard_propagate_wire_new_value(Gate instance, uint out_index, Wire wire, LxcValue value)
{
	Tokenport* ports = wire->drivens;
	Signal signal = wire->type;
	int len = wire->drivens_length;

	int i;
	for(i=0;i<len;++i)
	{
		Tokenport p = ports[i];
		if(NULL == p)
		{
			return;
		}

		lxc_import_new_value(value, (LxcValue*) &(p->current_value));

		if(p->gate->enabled && NULL != p->gate->execution_behavior)
		{
			p->gate->execution_behavior(p->gate, signal, wire->subtype, value, p->index);
		}
	}
}


static bool wire_nonhazard_propagate_availabe(Tokenport p)
{
	return NULL != p->current_value;
}

static void wire_nonhazard_propagate_absorb(Tokenport tp)
{
	lxc_import_new_value(NULL, (LxcValue*) &(tp->current_value));
}

static void wire_nonhazard_propagate_noop(Tokenport tp)
{}

static LxcValue wire_nonhazard_propagate_get_value(Tokenport tp)
{
	return (LxcValue) tp->current_value;
}

struct wire_handler_logic NONHAZARD_PROPAGATE_VALUE =
{
	.write_new_value = wire_nonhazard_propagate_wire_new_value,

	.is_token_available = wire_nonhazard_propagate_availabe,
	.token_get_value = wire_nonhazard_propagate_get_value,
	.absorb_token = wire_nonhazard_propagate_absorb,
	.release_token = wire_nonhazard_propagate_noop,
};

static void test_scenario_bool_gate_circuit__without_hazard(void)
{
	logxcontroll_init_environment();
	IOCircuit circ = create_multistage_bool_network();

	struct puppet_gate_instance* driver = (struct puppet_gate_instance*)
		lxc_circuit_get_gate_by_refdes(circ, "network driver");

	struct puppet_gate_instance* receiver = (struct puppet_gate_instance*)
			lxc_circuit_get_gate_by_refdes(circ, "network sniffer");

	Wire input = lxc_circuit_get_wire_by_refdes(circ, "input");

	{
		struct obj_str_arr_pnt* a = lxc_circuit_get_all_wire_refdes(circ);
		int i = -1;
		while(NULL != a->array[++i])
		{
			Wire w = lxc_circuit_get_wire_by_refdes(circ, a->array[i]);
			TEST_ASSERT_NOT_NULL(w);
			w->handler  = &NONHAZARD_PROPAGATE_VALUE;
		}
		free(a->array);
		free(a);
	}

	lxc_circuit_set_all_gate_enable(circ, true);

	Gate drv = &driver->base.base;

	void*** result_array = (void***) &receiver->user_data;

	{
		array_pnt_init(result_array);
		lxc_drive_wire_value(drv, 0, input, lxc_bool_constant_value_false.value);
		assert_not_prelling_then_release_array(result_array);
	}

	{
		array_pnt_init(result_array);
		lxc_drive_wire_value(drv, 0, input, lxc_bool_constant_value_true.value);
		assert_not_prelling_then_release_array(result_array);
	}

	{
		array_pnt_init(result_array);
		lxc_drive_wire_value(drv, 0, input, lxc_bool_constant_value_false.value);
		assert_not_prelling_then_release_array(result_array);
	}

	{
		array_pnt_init(result_array);
		lxc_drive_wire_value(drv, 0, input, lxc_bool_constant_value_true.value);
		assert_not_prelling_then_release_array(result_array);
	}

	lxc_circuit_destroy(circ);

	logxcontroll_destroy_environment();

	//TODO we must not experience prelling
	//TODO wait for circuit become idle
}



#endif
