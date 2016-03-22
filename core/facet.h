/*
 * facet.h
 *
 *  Created on: 2016.03.22.
 *      Author: szupervigyor
 */

#ifndef FACET_H_
#define FACET_H_


/*********************** Value ASSOCIATED FACET FUNCTIONS *********************/

//atomically increment the underlying value's reference count.
int lxc_reference_value(LxcValue value);

//atomically decrement the underlying value's reference count,
//if reference count is 0 (or less), resource will be freed
//and current reference count returned.
int lxc_unference_value(LxcValue value);


LxcValue lxc_get_wire_value(Wire);

void* lxc_get_value(LxcValue);

/**
 * Generic value is a simple, reference counted type, which has a simple data
 * structure, so can be freed with a single free() call.
 * */
LxcValue lxc_create_generic_value(Signal, size_t);


LxcValue lxc_get_constant_by_name(char* name);

/******************** Signal ASSOCIATED FACET FUNCTION ************************/

Signal lxc_get_signal_by_name(const char*);

/******************* Wire/Wiring ASSOCIATED FACET FUNCTION ********************/

void lxc_drive_wire_value(Gate instance, uint out_index, Wire wire, LxcValue value);

int lxc_wire_gate_input(Signal type, Wire wire, Gate g, uint index);

int lxc_wire_gate_output(Signal type, Wire wire, Gate g, uint index);

Wire lxc_create_wire(Signal type);

/*********************** Gate ASSOCIATED FACET FUNCTION ***********************/

Gate lxc_new_instance_by_name(const char* name);

const char* lxc_get_gate_name(Gate);

int lxc_get_gate_input_types(Gate, Signal*, int max_length);

int lxc_get_gate_output_types(Gate, Signal*, int max_length);


int lxc_get_input_labels(Gate, const char**, int max_length);

int lxc_get_output_labels(Gate, const char**, int max_length);

int lxc_enumerate_properties(Gate, const char**, int max_length);

const char* lxc_get_property_label(Gate, const char*);

int lxc_get_property_value(Gate, const char*, char*,int);

int lxc_set_property_value(Gate, const char*, char*, char*, uint);


/*********************** .......... ASSOCIATED FACET FUNCTION *****************/

#endif /* FACET_H_ */
