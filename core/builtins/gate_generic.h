/*
 * gate_lazy.h
 *
 *  Created on: 2016.02.19.
 *      Author: szupervigyor
 */
#include "core/logxcontroll.h"

#ifndef GATE_LAZY_H_
#define GATE_LAZY_H_


/**
 * Manages the input or output ports
 *	this has one level complexity because this manages
 *	ports by type.
 *
 *	desing
 *
 *
 *	first index by Signal type O(n) operation,
 *		then use for access Signal specific:
 *			- sensitivity
 *			- label type
 *			- absolute wire index (used in port manager instance)
 *
 *	data access optimalisation ordered by freqvency
 *	- signal,int => sensitivity
 *	- signal,int => abs_wire_index
 *
 *
 *	siganl+index => abs_index
 *	abs_index => sensitivity
 *		wire_abs_index =>
 *		port_label =>
 *
 * */
struct lxc_port_manager
{
	bool sensitive_for_enable;
	//fix null terminated arrays
	//may not be null, if no element managed, it should contains an array whit
	//single NULL element


	Signal* managed_types;
	uint** to_abs;
	const char** port_names;
	uint* to_abs_size;

	bool* sensitivity;
};

bool lxc_port_check_portname_in_use(struct lxc_port_manager*, char* name);

void lxc_port_unchecked_add_new_port(struct lxc_port_manager*, char* port_name, Signal type, bool sensitive);

void lxc_port_init_port_manager_factory(struct lxc_port_manager* fact);

int lxc_port_get_absindex(struct lxc_port_manager*, Signal, uint);

int lxc_port_get_absindex_by_name(struct lxc_port_manager*, char* name);

struct lxc_port_registry
{
	struct lxc_port_manager* factory;
	Wire* wires;
};

/**
 * Basically there is two type of gate from the aspect,
 * 	where the port definition belongs to.
 *
 * 	ib: in Behavior: port definition are immutable, after once it created,
 * 		it will not modified. Then it's better to belong to the gate's behavior
 * 		and it can be constructed at the library load time.
 *
 *
 *	ii: in instance: port definitions are mutable, and can be different
 *		gate by gate. In this case every instance comes with own port definition
 *		generally, default port settings advised to initialized
 *		at gate creation time. port definitions can be modified by
 *		setting/modifying a gate property.
 * */

/********* port definitions In Behavior generic gate implementation ***********/

struct lxc_generic_ib_instance;

struct lxc_generic_ib_behavior
{
	struct lxc_gate_behavior base;
	const char* gate_name;
	int instance_memory_size;
	//struct lxc_generic_instance* (*create_instance)(const struct lxc_generic_behavior*);
	void (*instance_init)(struct lxc_generic_ib_instance*);
	void (*instance_destroy)(struct lxc_generic_ib_instance*);

	struct lxc_port_manager input_ports;

	struct lxc_port_manager output_ports;
};

struct lxc_generic_ib_instance
{
	struct lxc_instance base;
	Wire* inputs;
	Wire* outputs;
};

extern const struct lxc_generic_ib_behavior lxc_generic_ib_prototype;

void lxc_generic_ib_init(struct lxc_generic_ib_instance*);

Gate lxc_generic_ib_gate_create(const struct lxc_generic_ib_behavior*);

void lxc_generic_ib_gate_destroy(Gate);

const char* lxc_generic_get_ib_gate_name(Gate);

int lxc_generic_ib_get_input_types(Gate instance, Signal* arr, uint max_length);

const char* lxc_generic_ib_get_input_label(Gate instance, Signal signal, uint index);

int lxc_generic_ib_get_input_max_index(Gate instance, Signal type);

Wire lxc_generic_ib_get_input_wire(Gate instance, Signal type, uint index);

int lxc_generic_ib_wire_input(Gate instance, Signal signal, Wire wire, uint index);

void lxc_generic_ib_input_value_changed(Gate instance, Signal type, LxcValue value, uint index);

int lxc_generic_ib_get_output_types(Gate instance, Signal* arr, uint max_length);

const char* lxc_generic_ib_get_output_label(Gate instance, Signal signal, uint index);

int lxc_generic_ib_get_output_max_index(Gate instance, Signal type);

Wire lxc_generic_ib_get_output_wire(Gate instance, Signal type, uint index);

int lxc_generic_ib_wire_output(Gate instance, Signal signal, Wire wire, uint index);

const char* lxc_generic_ib_get_gate_name(Gate);





/********* port definitions In Instance generic gate implementation ***********/
struct lxc_generic_ii_instance;

struct lxc_generic_ii_behavior
{
	struct lxc_gate_behavior base;
	const char* gate_name;
	int instance_memory_size;

	void (*instance_init)(struct lxc_generic_ii_instance*);
	void (*instance_destroy)(struct lxc_generic_ii_instance*);
};

extern const struct lxc_generic_ii_behavior lxc_generic_ii_prototype;

struct lxc_generic_ii_instance
{
	struct lxc_instance base;
	struct lxc_port_manager input_ports;
	struct lxc_port_manager output_ports;
	Wire* inputs;
	Wire* outputs;
};


void lxc_generic_ii_init(struct lxc_generic_ii_instance*);

Gate lxc_generic_ii_gate_create(const struct lxc_generic_ii_behavior*);

void lxc_generic_ii_gate_destroy(Gate);

const char* lxc_generic_get_ii_gate_name(Gate);

int lxc_generic_ii_get_input_types(Gate instance, Signal* arr, uint max_length);

const char* lxc_generic_ii_get_input_label(Gate instance, Signal signal, uint index);

int lxc_generic_ii_get_input_max_index(Gate instance, Signal type);

Wire lxc_generic_ii_get_input_wire(Gate instance, Signal type, uint index);

int lxc_generic_ii_wire_input(Gate instance, Signal signal, Wire wire, uint index);

void lxc_generic_ii_input_value_changed(Gate instance, Signal type, LxcValue value, uint index);

int lxc_generic_ii_get_output_types(Gate instance, Signal* arr, uint max_length);

const char* lxc_generic_ii_get_output_label(Gate instance, Signal signal, uint index);

int lxc_generic_ii_get_output_max_index(Gate instance, Signal type);

Wire lxc_generic_ii_get_output_wire(Gate instance, Signal type, uint index);

int lxc_generic_ii_wire_output(Gate instance, Signal signal, Wire wire, uint index);

const char* lxc_generic_ii_get_gate_name(Gate);


/************************** Property Manager **********************************/

struct lxc_property
{
	char* name;
	char* description;
	int (*property_validator)(const char* name, const char* value, char* ret, int max_length);
};

struct lxc_property_manager
{
	struct lxc_property** properties;
	/*int (*property_operation)(const char* name, bool direction, char* ret, int max_length);*/
	void* (*access_property)(Gate, const char*);
};

void lxc_add_property
(
	struct lxc_property_manager*,
	const char* name,
	const char* description,
	int (*property_validator)(const char* name, const char* value, char* ret, int max_length)
);


















#endif /* GATE_LAZY_H_ */
