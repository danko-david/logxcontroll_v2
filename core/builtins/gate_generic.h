/*
 * gate_lazy.h
 *
 *  Created on: 2016.02.19.
 *      Author: szupervigyor
 */
#include "core/logxcontroll.h"

#ifndef GATE_LAZY_H_
#define GATE_LAZY_H_

struct lxc_full_signal_type
{
	Signal signal;
	int subtype;
};

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
	//fix null terminated arrays
	//may not be null, if no element managed, it should contains an array whit
	//single NULL element

	int max_size;

	struct lxc_full_signal_type* managed_types;

	uint** to_abs;
	const char** port_names;

	//size of managed ports by Signal type (managed_types)
	uint* to_abs_size;
};

extern const char* lxc_port_empty_name;

bool lxc_port_check_portname_in_use(struct lxc_port_manager*, const char* name);

int lxc_port_unchecked_add_new_port
(
	struct lxc_port_manager*,
	const char* port_name,
	Signal type,
	int subtype,
	int* index_in_type_group
);

void lxc_port_init_port_manager_factory(struct lxc_port_manager* fact);

int lxc_port_get_absindex(struct lxc_port_manager*, Signal, int, uint);

void lxc_port_get_type_and_index_by_absindex
(
	struct lxc_port_manager* fact,
	uint absindex,
	Signal* type,
	int* subtype,
	int* managed_type_index,
	int* index
);

void lxc_port_remove_port
(
	struct lxc_port_manager* factory,
	Signal type,
	int subtype,
	uint index
);

bool lxc_port_is_any_wire_connected
(
	struct lxc_port_manager* mngr,
	void** /*Wire or Tokenport*/ wires,
	uint max_length
);

void lxc_port_wipe_all(struct lxc_port_manager* fact);

int lxc_port_count(struct lxc_port_manager* factory);

int lxc_port_get_absindex_by_name(struct lxc_port_manager*, const char* name);

struct lxc_port_registry
{
	struct lxc_port_manager* factory;
	Tokenport* wires;
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

struct lxc_generic_portb_instance;

/**
 * Have to specify/implement:
 * 	required:
 * 		.base.instance_memory_size
 *		.base.execute
 *
 *
 * 	optionally:
 * 		.instance_init
 * 		.instance_destroy
 *		.base.enumerate_properties
 *
 *		.base.get_property_label
 *		.base.get_property_description
 *		.base.get_property_value
 *		.base.set_property
 *		.base.gatectl
 * */
struct lxc_generic_portb_behavior
{
	struct lxc_gate_behavior base;
	int instance_memory_size;
	void (*instance_init)(struct lxc_generic_portb_instance*);
	void (*instance_destroy)(struct lxc_generic_portb_instance*);

	struct lxc_port_manager input_ports;

	struct lxc_port_manager output_ports;
};

struct lxc_generic_portb_instance
{
	struct lxc_instance base;
	Tokenport* inputs;
	Wire* outputs;
};

extern const struct lxc_generic_portb_behavior lxc_generic_portb_prototype;

/********* port definitions In Instance generic gate implementation ***********/
struct lxc_generic_porti_instance;

/**
 * Have to specify/implement:
 * 	required:
 * 		.base.instance_memory_size
 *		.base.execute
 *
 *
 * 	optionally:
 * 		.instance_init
 * 		.instance_destroy
 *		.base.enumerate_properties
 *
 *		.base.get_property_label
 *		.base.get_property_description
 *		.base.get_property_value
 *		.base.set_property
 *		.base.gatectl
 * */
struct lxc_generic_porti_behavior
{
	struct lxc_gate_behavior base;
	uint instance_memory_size;

	void (*instance_init)(struct lxc_generic_porti_instance*);
	void (*instance_destroy)(struct lxc_generic_porti_instance*);
};

extern const struct lxc_generic_porti_behavior lxc_generic_porti_prototype;

struct lxc_generic_porti_instance
{
	struct lxc_instance base;
	struct lxc_port_manager input_ports;
	struct lxc_port_manager output_ports;
	uint inputs_length;
	Tokenport* inputs;
	uint outputs_length;
	Wire* outputs;
};

/************************** Property Manager **********************************/

struct lxc_property
{
	const char* name;
	const char* label;
	const char* description;
	const char* default_value;
	int (*property_validator)(Gate instance, bool direction, void* addr, const char* name, const char* value, char* ret, int max_length);
};

struct lxc_property_manager
{
	struct lxc_property** properties;
	void* (*access_property)(Gate, const char*);
};

int lxc_add_property
(
	struct lxc_property_manager* mngr,
	const char* name,
	const char* label,
	const char* description,
	const char* default_value,
	int (*property_operation)(Gate instance, bool direction, void* addr, const char* name, const char* value, char* ret, int max_length)
);


struct lxc_generic_portb_propb_behavior
{
	struct lxc_generic_portb_behavior base;
	struct lxc_property_manager properties;
};

/**
 * Have to specify/implement:
 * 	required:
 		.base.instance_memory_size
		.base.execute
		.properties.access_property

 * 	optionally:
 		.instance_init
 		.instance_destroy
		.base.gatectl
		.properties.notify_property_changed
 *
 * */
extern struct lxc_generic_portb_propb_behavior lxc_generic_portb_propb_prototype;

struct lxc_generic_porti_propb_behavior
{
	struct lxc_generic_porti_behavior base;
	struct lxc_property_manager properties;
};

extern struct lxc_generic_porti_propb_behavior lxc_generic_porti_propb_prototype;

int lxc_portb_get_absindex
(
	struct lxc_generic_portb_instance* gate,
	bool direction,
	Signal type,
	int subtype,
	uint index
);

void lxc_init_generic_library();

void lxc_init_from_prototype(void* dst, size_t dst_len, void* src, size_t src_len);


struct lxc_generic_value
{
	struct lxc_value base;
	int refcount;
	size_t size;
	char data[0];
};

/**
 * Generic value is a simple, reference counted type, which has a simple data
 * structure, so can be freed with a single free() call.
 * */
LxcValue lxc_create_generic_value(Signal, size_t);

#endif /* GATE_LAZY_H_ */
