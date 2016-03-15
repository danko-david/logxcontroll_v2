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

	//size of managed ports by Signal type (managed_types)
	uint* to_abs_size;

	bool* sensitivity;
};

extern const char* lxc_port_empty_name;

bool lxc_port_check_portname_in_use(struct lxc_port_manager*, char* name);

void lxc_port_unchecked_add_new_port(struct lxc_port_manager*, char* port_name, Signal type, bool sensitive);

void lxc_port_init_port_manager_factory(struct lxc_port_manager* fact);

int lxc_port_get_absindex(struct lxc_port_manager*, Signal, uint);

void lxc_port_get_type_and_index_by_absindex
(
	struct lxc_port_manager* fact,
	uint absindex,
	Signal* type,
	int* managed_type_index,
	int* index
);

void lxc_port_remove_port
(
	struct lxc_port_manager* factory,
	Signal type,
	uint index
);

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

struct lxc_generic_portb_instance;

struct lxc_generic_portb_behavior
{
	struct lxc_gate_behavior base;
	const char* gate_name;
	int instance_memory_size;
	//struct lxc_generic_instance* (*create_instance)(const struct lxc_generic_behavior*);
	void (*instance_init)(struct lxc_generic_portb_instance*);
	void (*instance_destroy)(struct lxc_generic_portb_instance*);

	struct lxc_port_manager input_ports;

	struct lxc_port_manager output_ports;
};

struct lxc_generic_portb_instance
{
	struct lxc_instance base;
	Wire* inputs;
	Wire* outputs;
};

extern const struct lxc_generic_portb_behavior lxc_generic_portb_prototype;

/*
void lxc_generic_portb_init(struct lxc_generic_portb_instance*);

Gate lxc_generic_portb_gate_create(const struct lxc_generic_portb_behavior*);

void lxc_generic_portb_gate_destroy(Gate);

const char* lxc_generic_get_portb_gate_name(Gate);

int lxc_generic_portb_get_input_types(Gate instance, Signal* arr, uint max_length);

const char* lxc_generic_portb_get_input_label(Gate instance, Signal signal, uint index);

int lxc_generic_portb_get_input_max_index(Gate instance, Signal type);

Wire lxc_generic_portb_get_input_wire(Gate instance, Signal type, uint index);

int lxc_generic_portb_wire_input(Gate instance, Signal signal, Wire wire, uint index);

void lxc_generic_portb_input_value_changed(Gate instance, Signal type, LxcValue value, uint index);

int lxc_generic_portb_get_output_types(Gate instance, Signal* arr, uint max_length);

const char* lxc_generic_portb_get_output_label(Gate instance, Signal signal, uint index);

int lxc_generic_portb_get_output_max_index(Gate instance, Signal type);

Wire lxc_generic_portb_get_output_wire(Gate instance, Signal type, uint index);

int lxc_generic_portb_wire_output(Gate instance, Signal signal, Wire wire, uint index);

const char* lxc_generic_portb_get_gate_name(Gate);

*/



/********* port definitions In Instance generic gate implementation ***********/
struct lxc_generic_porti_instance;

struct lxc_generic_porti_behavior
{
	struct lxc_gate_behavior base;
	const char* gate_name;
	int instance_memory_size;

	void (*instance_init)(struct lxc_generic_porti_instance*);
	void (*instance_destroy)(struct lxc_generic_porti_instance*);
};

extern const struct lxc_generic_porti_behavior lxc_generic_porti_prototype;

struct lxc_generic_porti_instance
{
	struct lxc_instance base;
	struct lxc_port_manager input_ports;
	struct lxc_port_manager output_ports;
	Wire* inputs;
	Wire* outputs;
};

/*
void lxc_generic_porti_init(struct lxc_generic_porti_instance*);

Gate lxc_generic_porti_gate_create(const struct lxc_generic_porti_behavior*);

void lxc_generic_porti_gate_destroy(Gate);

const char* lxc_generic_get_porti_gate_name(Gate);

int lxc_generic_porti_get_input_types(Gate instance, Signal* arr, uint max_length);

const char* lxc_generic_porti_get_input_label(Gate instance, Signal signal, uint index);

int lxc_generic_porti_get_input_max_index(Gate instance, Signal type);

Wire lxc_generic_porti_get_input_wire(Gate instance, Signal type, uint index);

int lxc_generic_porti_wire_input(Gate instance, Signal signal, Wire wire, uint index);

void lxc_generic_porti_input_value_changed(Gate instance, Signal type, LxcValue value, uint index);

int lxc_generic_porti_get_output_types(Gate instance, Signal* arr, uint max_length);

const char* lxc_generic_porti_get_output_label(Gate instance, Signal signal, uint index);

int lxc_generic_porti_get_output_max_index(Gate instance, Signal type);

Wire lxc_generic_porti_get_output_wire(Gate instance, Signal type, uint index);

int lxc_generic_porti_wire_output(Gate instance, Signal signal, Wire wire, uint index);

const char* lxc_generic_porti_get_gate_name(Gate);
*/


/************************** Property Manager **********************************/

struct lxc_property
{
	const char* name;
	const char* label;
	const char* description;
	int (*property_operation)(Gate instance, bool direction, void* addr, const char* name, const char* value, char* ret, int max_length);
};

struct lxc_property_manager
{
	struct lxc_property** properties;
	void* (*access_property)(Gate, const char*);
	void (*notify_property_changed)(Gate,char* property);
};

int lxc_add_property
(
	struct lxc_property_manager*,
	const char* name,
	const char* description,
	int (*property_operation)(Gate instance, bool direction, void* addr, const char* name, const char* value, char* ret, int max_length)
);


struct lxc_generic_portb_propb_behavior
{
	struct lxc_generic_portb_behavior base;
	struct lxc_property_manager properties;
};

extern struct lxc_generic_portb_propb_behavior lxc_generic_portb_propb_prototype;

/*
int generic_portb_propb_enumerate_properties(Gate instance, const char** arr, uint max_index);
const char* generic_portb_propb_get_property_label(Gate instance, char* property);
const char* generic_portb_propb_get_property_description(Gate instance, char* property);
int generic_portb_propb_get_property_value(Gate instance, char* property, char* dst, uint max_length);
int generic_portb_propb_set_property(Gate instance, char* property, char* value, char* err, uint max_length);
*/

struct lxc_generic_porti_propb_behavior
{
	struct lxc_generic_porti_behavior base;
	struct lxc_property_manager properties;
};

extern struct lxc_generic_porti_propb_behavior lxc_generic_porti_propb_prototype;


void lxc_init_generic_library();

void lxc_init_from_prototype(void* dst, size_t dst_len, void* src, size_t src_len);




#endif /* GATE_LAZY_H_ */
