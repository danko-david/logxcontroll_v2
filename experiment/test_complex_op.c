/*
 * test_complex_op.c
 *
 *  Created on: 2017.05.18.
 *      Author: szupervigyor
 *
 * Some test to describe functionality.
 */

#include "test-core/test_core.h"

/************************ PREVIOUS implentetion area **************************/

Wire lxc_create_primitive_wire(Signal signal)
{
	return lxc_create_wire(signal);
}

Gate lxc_create_gate_by_name(const char* name)
{
	struct lxc_gate_behavior* b = get_gate_entry_by_name(name);
	if(NULL == b)
	{
		return NULL;
	}

	return lxc_new_instance_by_behavior(b);
}

typedef int refcount_t;

struct obj_str_arr_pnt
{
	void* rtt_type;
	refcount_t refcount;
	void** array;
};

int lxc_wire_set_refdes(Wire w, const char* name)
{
	//TODO check renaming does'nt volatiles workspace's refdes uniqueness
	//lock workspace for modifiaction
	if(w->ref_des)
	{
		free(w->ref_des);
	}
	w->ref_des = copy_string(name);

	return 0;
}

struct obj_str_arr_pnt*  lxc_circuit_get_all_wire_refdes(IOCircuit circ)
{
	struct obj_str_arr_pnt* ret = malloc(sizeof(struct obj_str_arr_pnt));
	ret->refcount = 1;
	array_pnt_init(&ret->array);

	{
		int i = 0;
		while(NULL != circ->wires[i])
		{
			array_pnt_append_element(&ret->array, circ->wires[i]->ref_des);
			++i;
		}
	}
}

Wire lxc_circuit_get_wire_by_refdes(IOCircuit circ, const char* name)
{
	if(NULL == circ->wires)
	{
		return NULL;
	}

	int i = 0;
	while(NULL != circ->wires[i])
	{
		if(0 == strcmp(name, circ->wires[i]->ref_des))
		{
			return circ->wires[i];
		}
		++i;
	}

	return NULL;
}

bool lxc_circuit_add_wire(IOCircuit circ, Wire w)
{
	NP_ASSERT_NOT_NULL(circ);
	NP_ASSERT_NOT_NULL(w);
	NP_ASSERT_NOT_NULL(w->ref_des);

	if(NULL != lxc_circuit_get_wire_by_refdes(circ, w->ref_des))
	{
		return LXC_ERROR_ENTITY_BY_NAME_ALREADY_REGISTERED;
	}

	return -1 != array_pnt_append_element
	(
		(void***) &circ->wires,
		(void*) w
	);
}

int lxc_gate_set_refdes(Gate gate, const char* name)
{
	//TODO check renaming does'nt volatiles workspace's refdes uniqueness
	//lock workspace for modifiaction
	if(gate->ref_des)
	{
		free(gate->ref_des);
	}

	gate->ref_des = copy_string(name);

	return 0;
}

int lxc_circuit_add_gate(IOCircuit circ, Gate gate)
{
	if(NULL != lxc_circuit_get_wire_by_refdes(circ, gate->ref_des))
	{
		return LXC_ERROR_ENTITY_BY_NAME_ALREADY_REGISTERED;
	}

	array_pnt_append_element
	(
		(void***) &circ->gates,
		(void*) gate
	);

	return 0;
}

int lxc_circuit_set_name(IOCircuit circ, const char* name)
{
	if(NULL != circ->name)
	{
		free(circ->name);
	}
	circ->name = copy_string(name);
	return 0;
}

bool lxc_check_gate_exists(const char* name)
{
	return NULL != get_gate_entry_by_name(name);
}

/************************ PREVIOUS implentetion area **************************/

Wire add_new_primitive_wire_to_circuit
(
	IOCircuit circuit,
	Signal type,
	const char* reference_designator
)
{
	Wire wire = lxc_create_primitive_wire(type);
	NP_ASSERT_NOT_NULL(wire);
	NP_ASSERT_EQUAL(0, lxc_wire_set_refdes(wire, reference_designator));
	NP_ASSERT_EQUAL(true, lxc_circuit_add_wire(circuit, wire));
	NP_ASSERT_PTR_EQUAL(wire, lxc_circuit_get_wire_by_refdes(circuit, reference_designator));
	return wire;
}

Gate add_new_gate_to_circuit
(
	IOCircuit circuit,
	const char* gate_name,
	const char* reference_designator
)
{
	Gate gate = lxc_create_gate_by_name(gate_name);
	NP_ASSERT_NOT_NULL(gate);
	NP_ASSERT_EQUAL(0, lxc_gate_set_refdes(gate, reference_designator));
	NP_ASSERT_EQUAL(0, lxc_circuit_add_gate(circuit, gate));
	return gate;
}

void wiring_input(Gate gate, int index, Wire w)
{
	NP_ASSERT_NOT_NULL(gate);
	NP_ASSERT_NOT_NULL(w);
	NP_ASSERT_EQUAL(0, lxc_wire_gate_input(w->type, w->subtype, w, gate, index));
	Tokenport tp = gate->behavior->get_input_wire(gate, w->type, w->subtype, index);
	NP_ASSERT_NOT_NULL(tp);
	NP_ASSERT_PTR_EQUAL(tp->owner, w);
}

void wiring_output(Wire w, Gate gate, int index)
{
	NP_ASSERT_EQUAL(0, lxc_wire_gate_output(w->type, w->subtype, w, gate, index));
	Wire win = gate->behavior->get_output_wire(gate, w->type, w->subtype, index);
	NP_ASSERT_PTR_EQUAL(w, win);
}

void circuit_enable_all_gate(IOCircuit circ)
{
	{
		int i = 0;
		while(NULL != circ->gates[i])
		{
			lxc_gate_set_enabled(circ->gates[i], true);
			++i;
		}
	}
}

struct lxc_generic_porti_behavior PUPPET_GATE;
struct puppet_gate_instance
{
	struct lxc_generic_porti_instance base;
	void (*execute)(Gate instance, Signal type, int subtype, LxcValue value, uint index);
	void (*destroy_gate)(Gate);
	void* user_data;
};

void puppet_execute(Gate instance, Signal type, int subtype, LxcValue value, uint index)
{
	 struct puppet_gate_instance* gate = (struct puppet_gate_instance*) instance;
	 if(NULL != gate->execute)
	 {
		 gate->execute(instance, type, subtype, value, index);
	 }
}

struct puppet_gate_instance* create_puppet_gate()
{
	if(NULL == PUPPET_GATE.base.gate_name)
	{
		lxc_init_from_prototype
		(
			&PUPPET_GATE,
			sizeof(PUPPET_GATE),

			(void*)&lxc_generic_porti_prototype,
			sizeof(lxc_generic_porti_prototype)
		);

		char** name = (char**)&PUPPET_GATE.base.gate_name;
		*name = "PUPPET GATE";


		PUPPET_GATE.instance_memory_size = sizeof(struct puppet_gate_instance);
		PUPPET_GATE.base.execute = puppet_execute;

	}

	struct puppet_gate_instance* ret = (struct puppet_gate_instance*) lxc_new_instance_by_behavior(&PUPPET_GATE.base);
	return ret;
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
	NP_ASSERT_EQUAL(true, lxc_check_gate_exists("bool not"));
	NP_ASSERT_EQUAL(true, lxc_check_gate_exists("bool xor"));

	IOCircuit sub = lxc_create_circuit();
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
	//TODO wire up gates but another TODO for types
	return sub;
}

static void release_port_generic
(
	Gate gate,
	int (*enumerate_types)(Gate, Signal*, int*, uint),
	int (*max_of_type)(Gate, Signal, int),
	void* (*get_wire)(Gate, Signal, int, uint),
	int (*unwire)(Signal, int, Wire, Gate, uint)
)
{
	Signal sigs[20];
	int subs[20];
	int max = enumerate_types(gate, sigs, subs, 20);
	int i = 0;
	while(i < max)
	{
		int max = max_of_type(gate, sigs[i], subs[i]);
		int m = 0;
		while(m<max)
		{
			void* in = get_wire(gate, sigs[i], subs[i],  m);
			if(NULL != in)
			{
				//printf("unwiring: gate: %p, signal: %s, subtype: %d, index: %d\n", gate, sigs[i]->name, subs[i], m);
				NP_ASSERT_EQUAL(0, unwire(sigs[i], subs[i], NULL, gate,  m));
			}
			++m;
		}
		++i;
	}
}

static void release_all_port(Gate gate)
{
	release_port_generic
	(
		gate,
		gate->behavior->get_input_types,
		gate->behavior->get_input_max_index,
		gate->behavior->get_input_wire,
		lxc_wire_gate_input
	);



	release_port_generic
	(
		gate,
		gate->behavior->get_output_types,
		gate->behavior->get_output_max_index,
		gate->behavior->get_output_wire,
		lxc_wire_gate_output
	);
}


static void destroy_circuit(IOCircuit circ)
{
	//minimal naive implementation

	{//release gates
		if(NULL != circ->gates)
		{
			int i = 0;
			while(NULL != circ->gates[i])
			{
				release_all_port(circ->gates[i]);
				++i;
			}
		}
	}


	{
		int i = 0;
		while(NULL != circ->wires[i])
		{
			lxc_test_destroy_wire(circ->wires[i]);
			++i;
		}
		free(circ->wires);
	}

	{
		int i = 0;
		while(NULL != circ->gates[i])
		{
			free(circ->gates[i]->ref_des);

			circ->gates[i]->behavior->destroy(circ->gates[i]);
			++i;
		}

		free(circ->gates);
	}

	free(circ->name);
	free(circ);
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
	NP_ASSERT_EQUAL(true, lxc_check_gate_exists(name));

	IOCircuit sub = lxc_create_circuit();
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

		NP_ASSERT_EQUAL(0, lxc_circuit_add_gate(circ, &driver->base.base));
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

		NP_ASSERT_EQUAL(0, lxc_circuit_add_gate(circ, &receiver->base.base));
	}

	circuit_enable_all_gate(circ);

	int i=0;
	while(NULL != truth_table[i])
	{
		struct truth_table_entry* ent = truth_table[i];
		lxc_drive_wire_value
		(
			driver,
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
				driver,
				0,
				I2,
				ent->I2?
					lxc_bool_constant_value_true.value
				:
					lxc_bool_constant_value_false.value
			);
		}

		NP_ASSERT_NOT_NULL(receiver->user_data);
		bool* ret = (bool*) receiver->user_data;
		NP_ASSERT_EQUAL(ent->output, *ret);

		++i;
	}

	destroy_circuit(circ);
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


static void test_scenario_bool_gate_circuit__with_hazard(void)
{
	logxcontroll_init_environment();
	IOCircuit circ = create_multistage_bool_network();


	struct puppet_gate_instance* driver = create_puppet_gate();
	Wire input = lxc_circuit_get_wire_by_refdes(circ, "input");

	{
		lxc_gate_set_refdes(&driver->base.base, "network driver");
		lxc_port_unchecked_add_new_port
		(
			&driver->base.output_ports,
			"output",
			&lxc_signal_bool,
			0,
			NULL
		);

		NP_ASSERT_EQUAL(0, lxc_circuit_add_gate(circ, &driver->base.base));
		wiring_output(input, &driver->base.base, 0);
	}

	struct puppet_gate_instance* receiver = create_puppet_gate();
	{
		lxc_gate_set_refdes(&receiver->base.base, "network sniffer");
		receiver->execute = print_bool_value;
		lxc_port_unchecked_add_new_port
		(
			&receiver->base.input_ports,
			"input",
			&lxc_signal_bool,
			0,
			NULL
		);

		Wire output = lxc_circuit_get_wire_by_refdes(circ, "output");
		wiring_input(&receiver->base.base, 0, output);

		NP_ASSERT_EQUAL(0, lxc_circuit_add_gate(circ, &receiver->base.base));
	}

	circuit_enable_all_gate(circ);

	printf("writing new value\n");
	lxc_drive_wire_value(driver, 0, input, lxc_bool_constant_value_false.value);
	printf("writing new value\n");
	lxc_drive_wire_value(driver, 0, input, lxc_bool_constant_value_true.value);
	printf("writing new value\n");
	lxc_drive_wire_value(driver, 0, input, lxc_bool_constant_value_false.value);
	printf("writing new value\n");
	lxc_drive_wire_value(driver, 0, input, lxc_bool_constant_value_true.value);
	printf("=== END ===\n");

	/* TODO tester code:
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

	destroy_circuit(circ);

	//TODO destroy logxcontroll environment

}

/*static void test_scenario_bool_gate_circuit__without_hazard(void)
{
	logxcontroll_init_environment();
	IOCircuit circ = create_multistage_bool_network();

	struct obj_str_arr_pnt* wire_refdes = lxc_circuit_get_all_wire_refdes(circ);

	{
		int i=0;
		while(NULL != wire_refdes->array[i])
		{
			Wire w = lxc_circuit_get_wire_by_refdes(circ, wire_refdes->array[i]);

			//TODO set wires to token mode
			NP_ASSERT_NOT_NULL(w);
		}
	}

	//TODO wire_refdes destruction


	//TODO we must not experience prelling
	//TODO wait for circuit become idle


}
*/

