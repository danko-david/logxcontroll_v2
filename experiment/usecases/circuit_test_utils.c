/*
 * circuit_test_utils.c
 *
 *  Created on: 2017. júl. 11.
 *      Author: szupervigyor
 */

#include "core/logxcontroll.h"
#include "experiment/usecases/circuit_test_utils.h"

static void puppet_execute(Gate instance, Signal type, int subtype, LxcValue value, uint index)
{
	 struct puppet_gate_instance* gate = (struct puppet_gate_instance*) instance;
	 if(NULL != gate->execute)
	 {
		 gate->execute(instance, type, subtype, value, index);
	 }
}

bool unsafe_extract_boolean(LxcValue val)
{
	bool* ret = lxc_get_value(val);
	return *ret;
}

static void puppet_add_value_array_pnt(Gate instance, Signal type, int subtype, LxcValue value, uint index)
{
	struct puppet_gate_instance* p = (struct puppet_gate_instance*) instance;
	if(&lxc_signal_bool == type && 0 == subtype && NULL != value)
	{
		bool val = unsafe_extract_boolean(value);
		printf("Gate(%p) bool input (%d.) notified: %s\n", instance, index, val?"true":"false");
		array_pnt_append_element((void***)&p->user_data, value);
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

	struct puppet_gate_instance* ret = (struct puppet_gate_instance*) lxc_gate_create_by_behavior(&PUPPET_GATE.base);
	return ret;
}

IOCircuit create_basic_network_driver_sniffer_network()
{
	IOCircuit ret = lxc_circuit_create();

	struct puppet_gate_instance* driver = create_puppet_gate();

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

		TEST_ASSERT_EQUAL(0, lxc_circuit_add_gate(ret, &driver->base.base));

	}

	struct puppet_gate_instance* receiver = create_puppet_gate();
	{
		lxc_gate_set_refdes(&receiver->base.base, "network sniffer");
		receiver->execute = puppet_add_value_array_pnt;
		lxc_port_unchecked_add_new_port
		(
			&receiver->base.input_ports,
			"input",
			&lxc_signal_bool,
			0,
			NULL
		);

		TEST_ASSERT_EQUAL(0, lxc_circuit_add_gate(ret, &receiver->base.base));
	}

	return ret;
}

Wire add_new_primitive_wire_to_circuit
(
	IOCircuit circuit,
	Signal type,
	const char* reference_designator
)
{
	Wire wire = lxc_wire_create(type);
	TEST_ASSERT_NOT_NULL(wire);
	TEST_ASSERT_EQUAL(0, lxc_wire_set_refdes(wire, reference_designator));
	TEST_ASSERT_EQUAL(true, lxc_circuit_add_wire(circuit, wire));
	TEST_ASSERT_PTR_EQUAL(wire, lxc_circuit_get_wire_by_refdes(circuit, reference_designator));
	return wire;
}

Gate add_new_gate_to_circuit
(
	IOCircuit circuit,
	const char* gate_name,
	const char* reference_designator
)
{
	Gate gate = lxc_gate_create_by_name(gate_name);
	TEST_ASSERT_NOT_NULL(gate);
	TEST_ASSERT_EQUAL(0, lxc_gate_set_refdes(gate, reference_designator));
	TEST_ASSERT_EQUAL(0, lxc_circuit_add_gate(circuit, gate));
	return gate;
}

void wiring_input(Gate gate, int index, Wire w)
{
	TEST_ASSERT_NOT_NULL(gate);
	TEST_ASSERT_NOT_NULL(w);
	TEST_ASSERT_EQUAL(0, lxc_wire_gate_input(w->type, w->subtype, w, gate, index));
	Tokenport tp = gate->behavior->get_input_wire(gate, w->type, w->subtype, index);
	TEST_ASSERT_NOT_NULL(tp);
	TEST_ASSERT_PTR_EQUAL(tp->owner, w);
}

void wiring_output(Wire w, Gate gate, int index)
{
	TEST_ASSERT_EQUAL(0, lxc_wire_gate_output(w->type, w->subtype, w, gate, index));
	Wire win = gate->behavior->get_output_wire(gate, w->type, w->subtype, index);
	TEST_ASSERT_PTR_EQUAL(w, win);
}
