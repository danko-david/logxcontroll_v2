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


/*********************** Value ASSOCIATED FACET FUNCTIONS *********************/

//atomically increment the underlying value's reference count.
int lxc_reference_value(LxcValue value);

//atomically decrement the underlying value's reference count,
//if reference count is 0 (or less), resource will be freed
//and current reference count returned.
int lxc_unreference_value(LxcValue value);

int lxc_refdiff_value(LxcValue value, int count);


//TODO LxcValue lxc_get_wire_value(Wire);
LxcValue lxc_get_token_value(Tokenport);

void lxc_absorb_token(Tokenport);

void* lxc_get_value(LxcValue);

LxcValue lxc_get_constant_by_name(const char* name);

/******************** Signal ASSOCIATED FACET FUNCTION ************************/

Signal lxc_get_signal_by_name(const char*);

/******************* Wire/Wiring ASSOCIATED FACET FUNCTION ********************/

void lxc_drive_wire_value(Gate instance, uint out_index, Wire wire, LxcValue value);

int lxc_wire_gate_input(Signal type, int subtype, Wire wire, Gate g, uint index);

int lxc_wire_gate_output(Signal type, int subtype, Wire wire, Gate g, uint index);

Wire lxc_create_wire(Signal type);

/*********************** Gate ASSOCIATED FACET FUNCTION ***********************/

Gate lxc_new_instance_by_behavior(const struct lxc_gate_behavior*);

Gate lxc_new_instance_by_name(const char* name);

const char* lxc_get_gate_name(Gate);

bool lxc_gate_is_enabled(Gate);

void lxc_gate_set_enabled(Gate, bool);

int lxc_get_gate_input_types(Gate, Signal*, int*, int max_length);

int lxc_get_gate_output_types(Gate, Signal*, int*, int max_length);


int lxc_get_gate_input_max_index(Gate, Signal, int);

int lxc_get_gate_output_max_index(Gate, Signal, int);

const char* lxc_get_input_label(Gate gate, Signal type, int, uint index);

const char* lxc_get_output_label(Gate gate, Signal type, int, uint index);


Tokenport lxc_get_input_wire(Gate, Signal, int, uint);

int lxc_get_input_labels(Gate, const char**, int max_length);

int lxc_get_output_labels(Gate, const char**, int max_length);

Wire lxc_get_output_wire(Gate, Signal, int, uint);

int lxc_enumerate_properties(Gate, const char**, int max_length);

const char* lxc_get_property_description(Gate gate, const char* property);

const char* lxc_get_property_label(Gate, const char*);

int lxc_get_property_value(Gate, const char*, char*,int);

int lxc_set_property_value(Gate, const char*, const char*, char*, uint);

Signal lxc_get_signal_by_ordinal(int ordinal);

/*********************** .......... ASSOCIATED FACET FUNCTION *****************/

#endif /* FACET_H_ */
