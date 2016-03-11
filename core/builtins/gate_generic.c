/*
 * gate_lazy.c
 *
 *  Created on: 2016.02.19.
 *      Author: szupervigyor
 */

#include "core/logxcontroll.h"

/********************** Port Manager related functions ************************/
int lxc_port_get_absindex
(
	struct lxc_port_manager* fact,
	Signal type,
	uint index
)
{
	int tindex = -1;
	Signal* types = fact->managed_types;

	int i=0;
	Signal crnt = NULL;
	while(NULL != (crnt = *types))
	{
		if(type == crnt)
		{
			tindex = i;
			break;
		}

		++i;
		++types;
	}

	if(-1 == tindex)
		return -1;

	uint len = fact->to_abs_size[tindex];
	if(index < len)
		return fact->to_abs[tindex][tindex];

	return -1;
}

bool lxc_port_check_portname_in_use
(
	struct lxc_port_manager* fact,
	char* name
)
{
	const char** names = fact->port_names;
	while(NULL != *names)
	{
		if(strcmp(*names, name) == 0)
			return true;
	}

	return false;
}

/*
bool isset(char* array, int abs_index)
{
	return array[abs_index >> 4] & (0x1 << (abs_index & 7));
}

void bit_array_set(char* array, int abs_index, bool value)
{
	if(value)
		array[abs_index >> 4] &= (0x1 << (abs_index & 7));
	else
		array[abs_index >> 4] |= ~(0x1 << (abs_index & 7));
}
*/

void lxc_port_unchecked_add_new_port
(
	struct lxc_port_manager* factory,
	char* port_name,
	Signal type,
	bool sensitive
)
{
	//determine the next free absolute index
	int next_abs = array_pnt_population((void**)factory->port_names);

	//determine the index of the signal
	int ti = array_pnt_contains((void**)factory->managed_types, (void*)type);

	if(ti < 0)
	{
		//index in other array by type
		ti =	array_pnt_append_element
				(
					(void***)&(factory->managed_types),
					(void*)type
				);

		//for new signal types, we need a new to_abs array under the
		//specified specified index `ti` this array contains a single value,
		//the next highest abs index and the size
		//of the array should be registered under factory->to_abs_size[ti];

		//first i create a new empty array for new signal type and a new single
		//slot for the size, then in common operation i add them all.

		//the to_abs, sure it will be under the same index as ti!
		int** toabs = NULL;
		array_pnt_init((void***)&toabs);
		array_pnt_append_element((void***)&(factory->to_abs), (void*)toabs);
		factory->to_abs_size = realloc(factory->to_abs_size, (ti+1)*sizeof(uint*));
		factory->to_abs_size[ti] = 0;
	}

	//add the abs_index to the end of the array (of to_abs)
	//and increase the to_abs_size value by 1
	factory->to_abs[ti] =	realloc
							(
								factory->to_abs[ti],
								sizeof(uint*)*(factory->to_abs_size[ti]+1)
							);

	factory->to_abs[ti][factory->to_abs_size[ti]] = next_abs;
	++(factory->to_abs_size[ti]);

	//add the wire name to the end of the array
	array_pnt_append_element
	(
		(void***)&(factory->port_names),
		(void*) copy_string(port_name)
	);

	//register sensitivity
	factory->sensitivity = realloc(factory->sensitivity, sizeof(bool)*(next_abs+1));
	factory->sensitivity[next_abs] = sensitive;

}

int lxc_port_get_absindex_by_name(struct lxc_port_manager* fact, char* name)
{
	int i=0;
	const char** arr = fact->port_names;
	while(NULL != *arr)
	{
		printf("%s\r\n", *arr);
		if(strcmp(*arr, name) == 0)
			return i;

		++i;
		++arr;
	}

	return -1;
}

void lxc_port_init_port_manager_factory(struct lxc_port_manager* fact)
{
	memset(fact, 0, sizeof(struct lxc_port_manager));
	fact->sensitive_for_enable = true;
	array_pnt_init((void***)&(fact->managed_types));
	array_pnt_init((void***)&(fact->port_names));
	array_pnt_init((void***)&(fact->to_abs));
	array_pnt_init((void***)&(fact->to_abs_size));
}

int lxc_port_fill_types
(
	struct lxc_port_manager* fact,
	Signal* dst_arr,
	uint max_length
)
{
	Signal* src = fact->managed_types;
	uint popul = array_pnt_population((void**)src);
	if(max_length < popul)
		return -popul;


	for(uint i=0;i<popul;++i)
		dst_arr[i] = src[i];

	return popul;
}

Wire lxc_port_get_wire
(
	struct lxc_port_manager* fact,
	Signal type,
	uint index,
	Wire* arr
)
{
	int abs = lxc_port_get_absindex(fact, type, index);
	if(abs < 0)
		return NULL;

	return arr[abs];
}

int lxc_port_wiring
(
	struct lxc_port_manager* fact,
	Signal type,
	uint index,
	Wire* arr,
	Wire w
)
{
	int abs = lxc_port_get_absindex(fact, type, index);
	if(abs < 0)
		return LXC_ERROR_PORT_OUT_OF_RANGE;

	arr[abs] = w;

	return 0;
}

const char* lxc_generic_get_port_label
(
	struct lxc_port_manager* fact,
	Signal type,
	uint index
)
{
	int abs = lxc_port_get_absindex(fact, type, index);
	if(abs < 0)
		return NULL;

	return fact->port_names[abs];
}

int lxc_generic_get_type_max_index(struct lxc_port_manager* fact, Signal type)
{
	Signal* src = fact->managed_types;
	for(int i=0;NULL != src[i];++i)
		if(type == src[i])
			return fact->to_abs_size[i];

	return -1;
}

//void (*library_operation)(enum library_operation);
//const char* (*get_gate_name)(Gate);
//void (*execute)(Gate instance, Signal type, LxcValue value, uint index);
//int (*gatectl)(Gate instance, unsigned long request, ...);

//TODO invariantManager

/********* port definitions In Behavior generic gate implementation ***********/

const struct lxc_generic_ib_behavior lxc_generic_ib_prototype =
{
	.base.get_gate_name = lxc_generic_get_ib_gate_name,
	.base.create = lxc_generic_ib_gate_create,
	.base.destroy = lxc_generic_ib_gate_destroy,
	.base.get_input_types = lxc_generic_ib_get_input_types,
	.base.get_input_label = lxc_generic_ib_get_input_label,
	.base.get_input_max_index = lxc_generic_ib_get_input_max_index,
	.base.get_input_wire = lxc_generic_ib_get_input_wire,
	.base.wire_input = lxc_generic_ib_wire_input,
	.base.input_value_changed = lxc_generic_ib_input_value_changed,
	.base.get_output_types = lxc_generic_ib_get_output_types,
	.base.get_output_label = lxc_generic_ib_get_output_label,
	.base.get_output_max_index = lxc_generic_ib_get_output_max_index,
	.base.get_output_wire = lxc_generic_ib_get_output_wire,
	.base.wire_output = lxc_generic_ib_wire_output,

/*	int (*enumerate_properties)(Gate instance, char** arr, uint max_index);

	const char* (*get_property_label)(Gate instance, char* property);

	const char* (*get_property_description)(Gate instance, char* property);

	int (*get_property_value)(Gate instance, char* property, char* dst, uint max_length);

	int (*set_property)(Gate instance, char* property, char* value, char* err, uint max_length);
*/

};

const char* lxc_generic_get_ib_gate_name(Gate instance)
{
	return ((struct lxc_generic_ib_behavior*)(instance->behavior))->gate_name;
}


Gate lxc_generic_ib_gate_create(const struct lxc_generic_ib_behavior* behav)
{
	struct lxc_generic_ib_instance* ret = NULL;
	ret = malloc_zero(behav->instance_memory_size);

	lxc_init_instance((Gate)ret, &(behav->base));
	lxc_generic_ib_init(ret);

	if(NULL != behav->instance_init)
		behav->instance_init(ret);

	return (Gate)ret;
}

void lxc_generic_ib_init(struct lxc_generic_ib_instance* instance)
{
	int insize =	array_pnt_population
					(
						(void**)((struct lxc_generic_ib_behavior*)(instance->base.behavior))
							->input_ports.managed_types
					);
	instance->inputs = malloc(sizeof(Wire)*insize);
	memset(instance->inputs, 0, sizeof(Wire)*insize);

	int outsize =	array_pnt_population
					(
						(void**)((struct lxc_generic_ib_behavior*)(instance->base.behavior))
							->output_ports.managed_types
					);
	instance->outputs = malloc(sizeof(Wire)*outsize);
	memset(instance->outputs, 0, sizeof(Wire)*outsize);
}


void lxc_generic_ib_gate_destroy(Gate instance)
{
	struct lxc_generic_ib_instance* ins = (struct lxc_generic_ib_instance*) instance;
	void (*destroy)(struct lxc_generic_ib_instance*) =
			((struct lxc_generic_ib_behavior*)(instance->behavior))
									->instance_destroy;

	if(NULL != destroy)
		destroy(ins);

	free(ins->inputs);
	free(ins->outputs);
}

int lxc_generic_ib_get_input_types(Gate instance, Signal* arr, uint max_length)
{
	return	lxc_port_fill_types
			(
				&((struct lxc_generic_ib_behavior*)instance->behavior)->input_ports,
				arr,
				max_length
			);
}

const char* lxc_generic_ib_get_input_label(Gate instance, Signal signal, uint index)
{
	return	lxc_generic_get_port_label
			(
				&((struct lxc_generic_ib_behavior*)instance->behavior)->input_ports,
				signal,
				index
			);
}

int lxc_generic_ib_get_input_max_index(Gate instance, Signal type)
{
	return	lxc_generic_get_type_max_index
			(
				&((struct lxc_generic_ib_behavior*)instance->behavior)->input_ports,
				type
			);
}

Wire lxc_generic_ib_get_input_wire(Gate instance, Signal type, uint index)
{
	return	lxc_port_get_wire
			(
				&((struct lxc_generic_ib_behavior*)instance->behavior)->input_ports,
				type,
				index,
				((struct lxc_generic_ib_instance*) instance)->inputs
			);
}

int lxc_generic_ib_wire_input(Gate instance, Signal type, Wire wire, uint index)
{
	return	lxc_port_wiring
			(
				&((struct lxc_generic_ib_behavior*)instance->behavior)->input_ports,
				type,
				index,
				((struct lxc_generic_ib_instance*) instance)->inputs,
				wire
			);
}

void lxc_generic_ib_input_value_changed
(
	Gate instance,
	Signal type,
	LxcValue value,
	uint index
)
{
	if(NULL == type)
		goto submit;

	int abs =	lxc_port_get_absindex
				(
					&((struct lxc_generic_ib_behavior*)instance->behavior)->input_ports,
					type,
					index
				);

	if(abs < 0)
		return;

		if
		(
			((struct lxc_generic_ib_behavior*) instance->behavior)->
				input_ports.sensitivity[abs]
		)
			goto submit;
		else
			return;

submit:
	instance->execution_behavior(instance, type, value, index);
}

int lxc_generic_ib_get_output_types(Gate instance, Signal* arr, uint max_length)
{
	return	lxc_port_fill_types
			(
				&((struct lxc_generic_ib_behavior*)instance->behavior)->output_ports,
				arr,
				max_length
			);
}

const char* lxc_generic_ib_get_output_label(Gate instance, Signal signal, uint index)
{
	return	lxc_generic_get_port_label
			(
				&((struct lxc_generic_ib_behavior*)instance->behavior)->output_ports,
				signal,
				index
			);
}

int lxc_generic_ib_get_output_max_index(Gate instance, Signal type)
{
	return	lxc_generic_get_type_max_index
			(
				&((struct lxc_generic_ib_behavior*)instance->behavior)->output_ports,
				type
			);
}

Wire lxc_generic_ib_get_output_wire(Gate instance, Signal type, uint index)
{
	return	lxc_port_get_wire
			(
				&((struct lxc_generic_ib_behavior*)instance->behavior)->output_ports,
				type,
				index,
				((struct lxc_generic_ib_instance*) instance)->outputs
			);
}

int lxc_generic_ib_wire_output(Gate instance, Signal type, Wire wire, uint index)
{
	return	lxc_port_wiring
			(
				&((struct lxc_generic_ib_behavior*)(instance->behavior))->output_ports,
				type,
				index,
				((struct lxc_generic_ib_instance*) instance)->outputs,
				wire
			);
}

/********* port definitions In Instance generic gate implementation ***********/

const struct lxc_generic_ii_behavior lxc_generic_ii_prototype =
{
	.base.get_gate_name = lxc_generic_ii_get_gate_name,
	.base.create = lxc_generic_ii_gate_create,
	.base.destroy = lxc_generic_ii_gate_destroy,
	.base.get_input_types = lxc_generic_ii_get_input_types,
	.base.get_input_label = lxc_generic_ii_get_input_label,
	.base.get_input_max_index = lxc_generic_ii_get_input_max_index,
	.base.get_input_wire = lxc_generic_ii_get_input_wire,
	.base.wire_input = lxc_generic_ii_wire_input,
	.base.input_value_changed = lxc_generic_ii_input_value_changed,
	.base.get_output_types = lxc_generic_ii_get_output_types,
	.base.get_output_label = lxc_generic_ii_get_output_label,
	.base.get_output_max_index = lxc_generic_ii_get_output_max_index,
	.base.get_output_wire = lxc_generic_ii_get_output_wire,
	.base.wire_output = lxc_generic_ii_wire_output,
};

const char* lxc_generic_ii_get_gate_name(Gate instance)
{
	return ((struct lxc_generic_ii_behavior*)(instance->behavior))->gate_name;
}



Gate lxc_generic_ii_gate_create(const struct lxc_generic_ii_behavior* behav)
{
	struct lxc_generic_ii_instance* ret = NULL;
	ret = malloc_zero(behav->instance_memory_size);

	lxc_init_instance((Gate)ret, &(behav->base));
	lxc_generic_ii_init(ret);

	if(NULL != behav->instance_init)
		behav->instance_init(ret);

	return (Gate)ret;
}

void lxc_generic_ii_init(struct lxc_generic_ii_instance* instance)
{
	int insize =	array_pnt_population
					(
						(void**)((struct lxc_generic_ii_instance*)instance)
							->input_ports.managed_types
					);
	instance->inputs = malloc(sizeof(Wire)*insize);
	memset(instance->inputs, 0, sizeof(Wire)*insize);

	int outsize =	array_pnt_population
					(
						(void**)((struct lxc_generic_ii_instance*)instance)
							->output_ports.managed_types
					);
	instance->outputs = malloc(sizeof(Wire)*outsize);
	memset(instance->outputs, 0, sizeof(Wire)*outsize);
}


void lxc_generic_ii_gate_destroy(Gate instance)
{
	struct lxc_generic_ii_instance* ins = (struct lxc_generic_ii_instance*) instance;
	void (*destroy)(struct lxc_generic_ii_instance*) =
			((struct lxc_generic_ii_behavior*)(instance->behavior))
									->instance_destroy;

	if(NULL != destroy)
		destroy(ins);

	free(ins->inputs);
	free(ins->outputs);
}

int lxc_generic_ii_get_input_types(Gate instance, Signal* arr, uint max_length)
{
	return	lxc_port_fill_types
			(
				&((struct lxc_generic_ii_instance*)instance)->input_ports,
				arr,
				max_length
			);
}

const char* lxc_generic_ii_get_input_label(Gate instance, Signal signal, uint index)
{
	return	lxc_generic_get_port_label
			(
				&(((struct lxc_generic_ii_instance*)instance)->input_ports),
				signal,
				index
			);
}

int lxc_generic_ii_get_input_max_index(Gate instance, Signal type)
{
	return	lxc_generic_get_type_max_index
			(
				&(((struct lxc_generic_ii_instance*)instance)->input_ports),
				type
			);
}

Wire lxc_generic_ii_get_input_wire(Gate instance, Signal type, uint index)
{
	return	lxc_port_get_wire
			(
				&(((struct lxc_generic_ii_instance*)instance)->input_ports),
				type,
				index,
				((struct lxc_generic_ii_instance*) instance)->inputs
			);
}

int lxc_generic_ii_wire_input(Gate instance, Signal type, Wire wire, uint index)
{
	return	lxc_port_wiring
			(
				&(((struct lxc_generic_ii_instance*)instance)->input_ports),
				type,
				index,
				((struct lxc_generic_ii_instance*) instance)->inputs,
				wire
			);
}

void lxc_generic_ii_input_value_changed
(
	Gate instance,
	Signal type,
	LxcValue value,
	uint index
)
{
	if(NULL == type)
		goto submit;

	int abs =	lxc_port_get_absindex
				(
					&((struct lxc_generic_ii_instance*)instance)->input_ports,
					type,
					index
				);

	if(abs < 0)
		return;

		if
		(
			((struct lxc_generic_ii_instance*) instance)->
				input_ports.sensitivity[abs]
		)
			goto submit;
		else
			return;

submit:
	instance->execution_behavior(instance, type, value, index);
}

int lxc_generic_ii_get_output_types(Gate instance, Signal* arr, uint max_length)
{
	return	lxc_port_fill_types
			(
				&(((struct lxc_generic_ii_instance*)instance)->output_ports),
				arr,
				max_length
			);
}

const char* lxc_generic_ii_get_output_label(Gate instance, Signal signal, uint index)
{
	return	lxc_generic_get_port_label
			(
				&((struct lxc_generic_ii_instance*)instance)->output_ports,
				signal,
				index
			);
}

int lxc_generic_ii_get_output_max_index(Gate instance, Signal type)
{
	return	lxc_generic_get_type_max_index
			(
				&((struct lxc_generic_ii_instance*)instance)->output_ports,
				type
			);
}

Wire lxc_generic_ii_get_output_wire(Gate instance, Signal type, uint index)
{
	return	lxc_port_get_wire
			(
				&((struct lxc_generic_ii_instance*)instance)->output_ports,
				type,
				index,
				((struct lxc_generic_ii_instance*) instance)->outputs
			);
}

int lxc_generic_ii_wire_output(Gate instance, Signal type, Wire wire, uint index)
{
	return	lxc_port_wiring
			(
				&(((struct lxc_generic_ii_instance*)instance)->output_ports),
				type,
				index,
				((struct lxc_generic_ii_instance*) instance)->outputs,
				wire
			);
}
