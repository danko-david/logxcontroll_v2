/*
 * facet.h
 *
 *  Created on: 2016.03.22.
 *      Author: szupervigyor
 *
 * This `facet.h` is a collection of necessary functions, used to export for
 * other libraries to can act with this lib together.
 *
 * For this set of function is recommented to create language specific bindings.
 * For example see: this repository/java-bindings
 */

#ifndef FACET_H_
#define FACET_H_

/******************** Behavior ASSOCIATED FACET FUNCTIONS *********************/
struct lxc_generic_portb_behavior* lxc_behavior_create_portb
(
	char* gate_name,
	int size,
	void (*execute)(Gate instance, Signal type, int subtype, LxcValue value, uint index)
);

struct lxc_generic_porti_behavior* behavior_create_porti
(
	char* gate_name,
	int size,
	void (*execute)(Gate instance, Signal type, int subtype, LxcValue value, uint index)
);


/*********************** Value ASSOCIATED FACET FUNCTIONS *********************/

//atomically increment the underlying value's reference count.
int lxc_reference_value(LxcValue value);

//atomically decrement the underlying value's reference count,
//if reference count is 0 (or less), resource will be freed
//and current reference count returned.
int lxc_unreference_value(LxcValue value);

int lxc_refdiff_value(LxcValue value, int count);

LxcValue lxc_get_token_value(Tokenport);

void lxc_absorb_token(Tokenport);

void lxc_wire_release_token(Tokenport tp);

void* lxc_get_value(LxcValue);

struct lxc_constant_value* lxc_get_constant_by_name(const char* name);

/******************** Signal ASSOCIATED FACET FUNCTION ************************/

Signal lxc_get_signal_by_name(const char*);

Signal lxc_get_signal_by_ordinal(int);

/******************* Wire/Wiring ASSOCIATED FACET FUNCTION ********************/

void lxc_drive_wire_value(Gate instance, uint out_index, Wire wire, LxcValue value);

int lxc_wire_add_debug_hook(Wire wire, struct lxc_wire_debug_hook_data* hook);

struct lxc_wire_debug_hook_data* lxc_wire_remove_debug_hook(Wire wire, const char* id);

struct lxc_wire_debug_hook_data* lxc_wire_get_debug_hook(Wire wire, const char* id);

int lxc_wire_gate_input(Signal type, int subtype, Wire wire, Gate g, uint index);

int lxc_wire_gate_output(Signal type, int subtype, Wire wire, Gate g, uint index);

int lxc_wire_set_refdes(Wire, const char*);

Wire lxc_wire_create(Signal type);

int lxc_wire_set_refdes(Wire, const char*);

bool lxc_wire_token_available(Tokenport tp);

/*********************** Gate ASSOCIATED FACET FUNCTION ***********************/

Gate lxc_gate_create_by_behavior(const struct lxc_gate_behavior*);

Gate lxc_gate_create_by_name(const char* name);

bool lxc_gate_exists(const char* name);

const char* lxc_gate_get_name(Gate);

bool lxc_gate_is_enabled(Gate);

void lxc_gate_set_enabled(Gate, bool);

int lxc_gate_get_input_types(Gate, Signal*, int*, int max_length);

int lxc_gate_get_output_types(Gate, Signal*, int*, int max_length);

int lxc_gate_get_input_max_index(Gate, Signal, int);

int lxc_gate_get_output_max_index(Gate, Signal, int);

const char* lxc_gate_get_input_label(Gate gate, Signal type, int, uint index);

const char* lxc_gate_get_output_label(Gate gate, Signal type, int, uint index);


Tokenport lxc_gate_get_input_port(Gate, Signal, int, uint);

int lxc_gate_get_input_labels(Gate, const char**, int max_length);

int lxc_gate_get_output_labels(Gate, const char**, int max_length);

Wire lxc_gate_get_output_wire(Gate, Signal, int, uint);

int lxc_gate_enumerate_properties(Gate, const char**, int max_length);

const char* lxc_gate_get_property_description(Gate gate, const char* property);

const char* lxc_gate_get_property_label(Gate, const char*);

int lxc_gate_get_property_value(Gate, const char*, char*,int);

int lxc_gate_set_property_value(Gate, const char*, const char*, char*, uint);

int lxc_gate_set_refdes(Gate, const char*);

void lxc_gate_enumerate_input_labels_into(const char*** dst, Gate g);
void lxc_gate_enumerate_output_labels_into(const char*** dst, Gate g);

/************************* Circuit ASSOCIATED FACET FUNCTION ******************/

IOCircuit lxc_circuit_create();

int lxc_circuit_add_gate_to(IOCircuit, Gate);

void lxc_circuit_destroy(IOCircuit);

int lxc_circuit_set_name(IOCircuit, const char*);

int lxc_circuit_add_gate(IOCircuit, Gate);

bool lxc_circuit_add_wire(IOCircuit, Wire);

Gate lxc_circuit_get_gate_by_refdes(IOCircuit, const char*);

Wire lxc_circuit_get_wire_by_refdes(IOCircuit, const char*);

void lxc_circuit_set_all_gate_enable(IOCircuit, bool);

Wire lxc_circuit_get_or_create_wire(IOCircuit, const char*, Signal);

/*********************** .......... ASSOCIATED FACET FUNCTION *****************/

#endif /* FACET_H_ */
