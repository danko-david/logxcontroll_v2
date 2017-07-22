/*
 * circuit_test_utils.h
 *
 *  Created on: 2017. júl. 11.
 *      Author: szupervigyor
 */

#ifndef REPOSITORY_EXPERIMENT_USECASES_CIRCUIT_TEST_UTILS_H_
#define REPOSITORY_EXPERIMENT_USECASES_CIRCUIT_TEST_UTILS_H_

struct lxc_generic_porti_behavior PUPPET_GATE;
struct puppet_gate_instance
{
	struct lxc_generic_porti_instance base;
	void (*execute)(Gate instance, Signal type, int subtype, LxcValue value, uint index);
	void (*destroy_gate)(Gate);
	void* user_data;
};

struct puppet_gate_instance* create_puppet_gate();

void wiring_input(Gate gate, int index, Wire w);

void wiring_output(Wire w, Gate gate, int index);

bool unsafe_extract_boolean(LxcValue val);

Gate add_new_gate_to_circuit
(
	IOCircuit circuit,
	const char* gate_name,
	const char* reference_designator
);

IOCircuit create_basic_network_driver_sniffer_network();

Wire add_new_primitive_wire_to_circuit
(
	IOCircuit circuit,
	Signal type,
	const char* reference_designator
);

#endif /* REPOSITORY_EXPERIMENT_USECASES_CIRCUIT_TEST_UTILS_H_ */
