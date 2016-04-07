/*
 * gate_lazy.c
 *
 *  Created on: 2016.02.19.
 *      Author: szupervigyor
 */

#include "core/logxcontroll.h"

/********************** Port Manager related functions ************************/

void lxc_port_get_type_and_index_by_absindex
(
	struct lxc_port_manager* fact,
	uint absindex,
	Signal* type,
	int* managed_type_index,
	int* index
)
{
	uint** to_abs = fact->to_abs;
	for(int i=0;NULL != to_abs[i];++i)
	{
		int max = fact->to_abs_size;
		uint* arr = to_abs[i];
		for(int a = 0;a < max;++a)
		{
			if(absindex == to_abs[a])
			{
				*type = fact->managed_types[i];
				*managed_type_index = i;
				*index = a;
				return;
			}
		}
	}

	*type = NULL;
	*managed_type_index = -1;
	*index = -1;
}


int lxc_port_get_absindex
(
	struct lxc_port_manager* fact,
	Signal type,
	uint index
)
{
	int tindex = -1;
	Signal* types = fact->managed_types;

	if(NULL == types)
	{
		return -1;
	}

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
		return fact->to_abs[tindex][index];

	return -1;
}

bool lxc_port_check_portname_in_use
(
	struct lxc_port_manager* fact,
	const char* name
)
{
	const char** names = fact->port_names;
	if(NULL == names)
	{
		return false;
	}

	for(int i=0;NULL != names[i];++i)
	{
		if(strcmp(names[i], name) == 0)
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

const char* lxc_port_empty_name = "";

int lxc_port_unchecked_add_new_port
(
	struct lxc_port_manager* factory,
	const char* port_name,
	Signal type,
	bool sensitive
)
{
	bool intermediate = false;

	const char** names = factory->port_names;

	int next_abs;

	if(NULL == names)
	{
		//fist add
		next_abs = 0;
	}
	else
	{
		//frist we try to find an empty slot
		for(int i=0;NULL != names[i];++i)
		{
			if(lxc_port_empty_name == names[i])
			{
				next_abs = i;
				intermediate = true;
				break;
			}
		}

		// if we doesn't find an empty slot (ie no intermediate replace will
		// be applied) we reserve a new one
		if(!intermediate)
		{
			//determine the next free absolute index
			next_abs = array_pnt_population((void**)factory->port_names);
		}
	}

	//determine the index of the signal
	int ti = array_pnt_contains((void**)factory->managed_types, (void*)type);

	if(ti < 0)
	{
		bool type_replace = false;
		Signal* mt = factory->managed_types;
		uint* lengths = factory->to_abs_size;
		if(NULL != mt && NULL!= lengths)
		{
			for(int i=0;NULL != mt[i] ;++i)
			{
				if(0 == lengths[i])
				{
					type_replace = true;
					ti = i;
					break;
				}
			}
		}

		//if there is no empty managed type
		//we allocate new slot for it
		if(!type_replace)
		{
			//index in other array by type
			ti =	array_pnt_append_element
					(
						(void***) &(factory->managed_types),
						(void*) type
					);
		}
		//for new signal types, we need a new to_abs array under the
		//specified index `ti`, this array contains a single value,
		//the next highest abs index and the size
		//of the array should be registered under factory->to_abs_size[ti];

		//first i create a new empty array for new signal type and a new single
		//slot for the size, then in common operation i add them all.

		//the to_abs, sure it will be under the same index as ti!
		if(!type_replace)
		{
			int** toabs = NULL;
			array_pnt_init((void***)&toabs);
			array_pnt_append_element((void***)&(factory->to_abs), (void*)toabs);
			factory->to_abs_size = realloc(factory->to_abs_size, (ti+1)*sizeof(uint*));
		}

		factory->to_abs_size[ti] = 0;
	}

	//anyway we have to growth the to_abs array for the new element
	//add the abs_index to the end of the array (of to_abs)
	//and increase the to_abs_size value by 1
	factory->to_abs[ti] =	realloc
							(
								factory->to_abs[ti],
								sizeof(uint*)*(factory->to_abs_size[ti]+1)
							);

	factory->to_abs[ti][factory->to_abs_size[ti]] = next_abs;
	++(factory->to_abs_size[ti]);

	if(intermediate)
	{
		factory->port_names[next_abs] = port_name;
	}
	else
	{
		//add the wire name to the end of the array
		array_pnt_append_element
		(
			(void***)&(factory->port_names),
			(void*) port_name
		);
	}

	//if not intermediate we put the value to an already exists place.
	if(!intermediate)
	{
		//register sensitivity
		factory->sensitivity = realloc(factory->sensitivity, sizeof(bool)*(next_abs+1));
	}

	factory->sensitivity[next_abs] = sensitive;

	factory->max_size = array_pnt_population((void**)factory->port_names);
	return next_abs;
}

int lxc_port_count(struct lxc_port_manager* factory)
{
	return factory->max_size;
}

void lxc_port_remove_port
(
	struct lxc_port_manager* factory,
	Signal type,
	uint index
)
{
	//remove from abs_index
	//port_names => address of identity char*
	//update to_abs_size

	Signal* sigs = factory->managed_types;
	int ti = -1;

	for(int i=0;NULL != sigs[i];++i)
	{
		if(type == sigs[i])
		{
			ti = i;
			break;
		}
	}

	if(-1 == ti)
		return;

	int abs = lxc_port_get_absindex(factory, type, index);
	if(abs < 0)
		return;

	factory->port_names[abs] = lxc_port_empty_name;
	--(factory->to_abs_size[ti]);
	array_pnt_pop_element((void***) &(factory->to_abs[ti]), index);

	factory->max_size = array_pnt_population((void**)factory->port_names);
}

bool lxc_port_is_any_wire_connected
(
	struct lxc_port_manager* mngr,
	Wire* wires,
	uint max_length
)
{
	if(NULL == wires)
	{
		return false;
	}

	int size = lxc_port_count(mngr);
	if(max_length < size)
	{
		size = max_length;
	}

	for(int i=0;i< size;++i)
	{
		 if(NULL != wires[i])
		 {
			 return true;
		 }
	}

	return false;
}

void lxc_port_wipe_all(struct lxc_port_manager* fact)
{
	if(NULL == fact)
	{
		return;
	}

	int ti_max = array_pnt_population(fact->managed_types);

	if(ti_max <= 0)
	{
		return;
	}

	free(fact->managed_types);
	free(fact->port_names);
	free(fact->sensitivity);

	for(int i=0;i<ti_max;++i)
	{
		free(fact->to_abs[i]);
	}

	free(fact->to_abs_size);

	memset(fact, 0, sizeof(struct lxc_port_manager));

	lxc_port_init_port_manager_factory(fact);
}

int lxc_port_get_absindex_by_name(struct lxc_port_manager* fact, const char* name)
{
	int i=0;
	const char** arr = fact->port_names;
	while(NULL != arr)
	{
		//printf("%s\r\n", *arr);
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

Wire lxc_port_get_wire_safe
(
	struct lxc_port_manager* fact,
	Signal type,
	uint index,
	Wire* arr,
	int port_size
)
{
	int abs = lxc_port_get_absindex(fact, type, index);
	if(abs < 0 || abs >= port_size)
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
		return LXC_ERROR_ENTITY_OUT_OF_RANGE;

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

/********* port definitions In Behavior generic gate implementation ***********/

static void lxc_generic_portb_init(struct lxc_generic_portb_instance* instance)
{
	struct lxc_generic_portb_behavior* b =
		(struct lxc_generic_portb_behavior*) instance->base.behavior;

	int insize = lxc_port_count(&(b->input_ports));
	instance->inputs = malloc_zero(sizeof(Wire)*insize);
	int outsize = lxc_port_count(&(b->output_ports));
	instance->outputs = malloc_zero(sizeof(Wire)*outsize);
}

static Gate lxc_generic_portb_gate_create(const struct lxc_generic_portb_behavior* behav)
{
	struct lxc_generic_portb_instance* ret = NULL;
	ret = malloc_zero(behav->instance_memory_size);

	lxc_init_instance((Gate)ret, &(behav->base));
	lxc_generic_portb_init(ret);

	if(NULL != behav->instance_init)
		behav->instance_init(ret);

	return (Gate)ret;
}


static void lxc_generic_portb_gate_destroy(Gate instance)
{
	struct lxc_generic_portb_instance* ins = (struct lxc_generic_portb_instance*) instance;
	void (*destroy)(struct lxc_generic_portb_instance*) =
			((struct lxc_generic_portb_behavior*)(instance->behavior))
									->instance_destroy;

	if(NULL != destroy)
		destroy(ins);

	free(ins->inputs);
	free(ins->outputs);
}

static int lxc_generic_portb_get_input_types(Gate instance, Signal* arr, uint max_length)
{
	return	lxc_port_fill_types
			(
				&((struct lxc_generic_portb_behavior*)instance->behavior)->input_ports,
				arr,
				max_length
			);
}

static const char* lxc_generic_portb_get_input_label(Gate instance, Signal signal, uint index)
{
	return	lxc_generic_get_port_label
			(
				&((struct lxc_generic_portb_behavior*)instance->behavior)->input_ports,
				signal,
				index
			);
}

static int lxc_generic_portb_get_input_max_index(Gate instance, Signal type)
{
	return	lxc_generic_get_type_max_index
			(
				&((struct lxc_generic_portb_behavior*)instance->behavior)->input_ports,
				type
			);
}

static Wire lxc_generic_portb_get_input_wire(Gate instance, Signal type, uint index)
{
	return	lxc_port_get_wire_safe
			(
				&((struct lxc_generic_portb_behavior*)instance->behavior)->input_ports,
				type,
				index,
				((struct lxc_generic_portb_instance*) instance)->inputs,
				((struct lxc_generic_portb_behavior*) instance->behavior)->input_ports.max_size
			);
}

static int lxc_generic_portb_wire_input(Gate instance, Signal type, Wire wire, uint index)
{
	return	lxc_port_wiring
			(
				&((struct lxc_generic_portb_behavior*)instance->behavior)->input_ports,
				type,
				index,
				((struct lxc_generic_portb_instance*) instance)->inputs,
				wire
			);
}

static void lxc_generic_portb_input_value_changed
(
	Gate instance,
	Signal type,
	LxcValue value,
	uint index
)
{
	if(&lxc_signal_system == type)
		goto submit;

	int abs =	lxc_port_get_absindex
				(
					&((struct lxc_generic_portb_behavior*)instance->behavior)->input_ports,
					type,
					index
				);

	if(abs < 0)
		return;

		if
		(
			((struct lxc_generic_portb_behavior*) instance->behavior)->
				input_ports.sensitivity[abs]
		)
			goto submit;
		else
			return;

submit:
	instance->execution_behavior(instance, type, value, index);
}

static int lxc_generic_portb_get_output_types(Gate instance, Signal* arr, uint max_length)
{
	return	lxc_port_fill_types
			(
				&((struct lxc_generic_portb_behavior*)instance->behavior)->output_ports,
				arr,
				max_length
			);
}

static const char* lxc_generic_portb_get_output_label(Gate instance, Signal signal, uint index)
{
	return	lxc_generic_get_port_label
			(
				&((struct lxc_generic_portb_behavior*)instance->behavior)->output_ports,
				signal,
				index
			);
}

static int lxc_generic_portb_get_output_max_index(Gate instance, Signal type)
{
	return	lxc_generic_get_type_max_index
			(
				&((struct lxc_generic_portb_behavior*)instance->behavior)->output_ports,
				type
			);
}

static Wire lxc_generic_portb_get_output_wire(Gate instance, Signal type, uint index)
{
	return	lxc_port_get_wire_safe
			(
				&((struct lxc_generic_portb_behavior*)instance->behavior)->output_ports,
				type,
				index,
				((struct lxc_generic_portb_instance*) instance)->outputs,
				((struct lxc_generic_portb_behavior*) instance->behavior)->output_ports.max_size
			);
}

static int lxc_generic_portb_wire_output(Gate instance, Signal type, Wire wire, uint index)
{
	return	lxc_port_wiring
			(
				&((struct lxc_generic_portb_behavior*)(instance->behavior))->output_ports,
				type,
				index,
				((struct lxc_generic_portb_instance*) instance)->outputs,
				wire
			);
}

int lxc_portb_get_absindex
(
	struct lxc_generic_portb_instance* gate,
	bool direction,
	Signal type,
	uint index
)
{
	struct lxc_generic_portb_behavior* b =
		(struct lxc_generic_portb_behavior*) (gate->base.behavior);
	return
		direction?
			lxc_port_get_absindex(&(b->input_ports), type, index)
		:
			lxc_port_get_absindex(&(b->output_ports), type, index);
}

const struct lxc_generic_portb_behavior lxc_generic_portb_prototype =
{
	.base.create = lxc_generic_portb_gate_create,
	.base.destroy = lxc_generic_portb_gate_destroy,
	.base.get_input_types = lxc_generic_portb_get_input_types,
	.base.get_input_label = lxc_generic_portb_get_input_label,
	.base.get_input_max_index = lxc_generic_portb_get_input_max_index,
	.base.get_input_wire = lxc_generic_portb_get_input_wire,
	.base.wire_input = lxc_generic_portb_wire_input,
	.base.input_value_changed = lxc_generic_portb_input_value_changed,
	.base.get_output_types = lxc_generic_portb_get_output_types,
	.base.get_output_label = lxc_generic_portb_get_output_label,
	.base.get_output_max_index = lxc_generic_portb_get_output_max_index,
	.base.get_output_wire = lxc_generic_portb_get_output_wire,
	.base.wire_output = lxc_generic_portb_wire_output,

/*	int (*enumerate_properties)(Gate instance, char** arr, uint max_index);

	const char* (*get_property_label)(Gate instance, char* property);

	const char* (*get_property_description)(Gate instance, char* property);

	int (*get_property_value)(Gate instance, char* property, char* dst, uint max_length);

	int (*set_property)(Gate instance, char* property, char* value, char* err, uint max_length);
*/

};


/********* port definitions In Instance generic gate implementation ***********/

static void lxc_generic_porti_init(struct lxc_generic_porti_instance* instance)
{
	int insize = lxc_port_count(&(instance->input_ports));
	instance->inputs = malloc_zero(sizeof(Wire)*insize);

	int outsize = lxc_port_count(&(instance->output_ports));
	instance->outputs = malloc_zero(sizeof(Wire)*outsize);
}

static Gate lxc_generic_porti_gate_create(const struct lxc_generic_porti_behavior* behav)
{
	struct lxc_generic_porti_instance* ret = NULL;
	ret = malloc_zero(behav->instance_memory_size);

	lxc_init_instance((Gate)ret, &(behav->base));
	lxc_generic_porti_init(ret);

	if(NULL != behav->instance_init)
		behav->instance_init(ret);

	return (Gate)ret;
}


static void lxc_generic_porti_gate_destroy(Gate instance)
{
	struct lxc_generic_porti_instance* ins = (struct lxc_generic_porti_instance*) instance;
	void (*destroy)(struct lxc_generic_porti_instance*) =
			((struct lxc_generic_porti_behavior*)(instance->behavior))
									->instance_destroy;

	if(NULL != destroy)
		destroy(ins);

	free(ins->inputs);
	free(ins->outputs);
}

static int lxc_generic_porti_get_input_types(Gate instance, Signal* arr, uint max_length)
{
	return	lxc_port_fill_types
			(
				&((struct lxc_generic_porti_instance*)instance)->input_ports,
				arr,
				max_length
			);
}

static const char* lxc_generic_porti_get_input_label(Gate instance, Signal signal, uint index)
{
	return	lxc_generic_get_port_label
			(
				&(((struct lxc_generic_porti_instance*)instance)->input_ports),
				signal,
				index
			);
}

static int lxc_generic_porti_get_input_max_index(Gate instance, Signal type)
{
	return	lxc_generic_get_type_max_index
			(
				&(((struct lxc_generic_porti_instance*)instance)->input_ports),
				type
			);
}

static Wire lxc_generic_porti_get_input_wire(Gate instance, Signal type, uint index)
{
	return	lxc_port_get_wire_safe
			(
				&(((struct lxc_generic_porti_instance*)instance)->input_ports),
				type,
				index,
				((struct lxc_generic_porti_instance*) instance)->inputs,
				((struct lxc_generic_porti_instance*) instance)->inputs_length
			);
}

static void force_index_available
(
	struct lxc_port_manager* mngr,
	Wire** array_addr,
	uint* array_length
)
{
	int need = lxc_port_count(mngr);
	if(need >= *array_length)
	{
		*array_addr =	realloc_zero
						(
							*array_addr,
							sizeof(void*)*(*array_length),
							sizeof(void*)*need
						);
		*array_length = need;
	}
}

static int lxc_generic_porti_wire_input(Gate instance, Signal type, Wire wire, uint index)
{
	struct lxc_generic_porti_instance* i = (struct lxc_generic_porti_instance*) instance;

	force_index_available
	(
		&(i->input_ports),
		&(i->inputs),
		&(i->inputs_length)
	);

	return	lxc_port_wiring
			(
				&(i->input_ports),
				type,
				index,
				i->inputs,
				wire
			);
}

static void lxc_generic_porti_input_value_changed
(
	Gate instance,
	Signal type,
	LxcValue value,
	uint index
)
{
	if(&lxc_signal_system == type)
		goto submit;

	int abs =	lxc_port_get_absindex
				(
					&((struct lxc_generic_porti_instance*)instance)->input_ports,
					type,
					index
				);

	if(abs < 0)
		return;

		if
		(
			((struct lxc_generic_porti_instance*) instance)->
				input_ports.sensitivity[abs]
		)
			goto submit;
		else
			return;

submit:
	instance->execution_behavior(instance, type, value, index);
}

static int lxc_generic_porti_get_output_types(Gate instance, Signal* arr, uint max_length)
{
	return	lxc_port_fill_types
			(
				&(((struct lxc_generic_porti_instance*)instance)->output_ports),
				arr,
				max_length
			);
}

static const char* lxc_generic_porti_get_output_label(Gate instance, Signal signal, uint index)
{
	return	lxc_generic_get_port_label
			(
				&((struct lxc_generic_porti_instance*)instance)->output_ports,
				signal,
				index
			);
}

static int lxc_generic_porti_get_output_max_index(Gate instance, Signal type)
{
	return	lxc_generic_get_type_max_index
			(
				&((struct lxc_generic_porti_instance*)instance)->output_ports,
				type
			);
}

static Wire lxc_generic_porti_get_output_wire(Gate instance, Signal type, uint index)
{
	return	lxc_port_get_wire_safe
			(
				&((struct lxc_generic_porti_instance*)instance)->output_ports,
				type,
				index,
				((struct lxc_generic_porti_instance*) instance)->outputs,
				((struct lxc_generic_porti_instance*) instance)->outputs_length
			);
}

static int lxc_generic_porti_wire_output(Gate instance, Signal type, Wire wire, uint index)
{
	struct lxc_generic_porti_instance* i = (struct lxc_generic_porti_instance*) instance;

	force_index_available
	(
		&(i->output_ports),
		&(i->outputs),
		&(i->outputs_length)
	);

	return	lxc_port_wiring
			(
				&(i->output_ports),
				type,
				index,
				i->outputs,
				wire
			);
}

const struct lxc_generic_porti_behavior lxc_generic_porti_prototype =
{
	.base.create = lxc_generic_porti_gate_create,
	.base.destroy = lxc_generic_porti_gate_destroy,
	.base.get_input_types = lxc_generic_porti_get_input_types,
	.base.get_input_label = lxc_generic_porti_get_input_label,
	.base.get_input_max_index = lxc_generic_porti_get_input_max_index,
	.base.get_input_wire = lxc_generic_porti_get_input_wire,
	.base.wire_input = lxc_generic_porti_wire_input,
	.base.input_value_changed = lxc_generic_porti_input_value_changed,
	.base.get_output_types = lxc_generic_porti_get_output_types,
	.base.get_output_label = lxc_generic_porti_get_output_label,
	.base.get_output_max_index = lxc_generic_porti_get_output_max_index,
	.base.get_output_wire = lxc_generic_porti_get_output_wire,
	.base.wire_output = lxc_generic_porti_wire_output,
};

/************************** Property Manager **********************************/


struct lxc_generic_portb_propb_behavior lxc_generic_portb_propb_prototype;


int lxc_add_property
(
	struct lxc_property_manager* mngr,
	const char* name,
	const char* label,
	const char* description,
	const char* default_value,
	int (*property_validator)(Gate instance, bool direction, void* addr, const char* name, const char* value, char* ret, int max_length)
)
{
	struct lxc_property** props = mngr->properties;
	if(NULL != props)
	{
		for(int i=0;NULL != props[i];++i)
			if(0 == strcmp(name, props[i]->name))
				return LXC_ERROR_ENTITY_ALREADY_REGISTERED;
	}

	struct lxc_property* prop = malloc_zero(sizeof(struct lxc_property));
	prop->name = name;
	prop->label = label;
	prop->description = description;
	prop->default_value = default_value;
	prop->property_validator = property_validator;

	array_pnt_append_element((void***) &(mngr->properties), prop);

	return 0;
}

//TODO remove property

static int generic_enumerate_properties(struct lxc_property_manager* mngr, const char** arr, uint max_index)
{
	int popul = array_pnt_population((void**)mngr->properties);

	if(0 == popul)
		return 0;

	if(max_index <= popul)
		return -popul;

	struct lxc_property** props = mngr->properties;

	for(int i=0;NULL != props[i];++i)
		arr[i] = props[i]->name;

	return popul;
}

static const char* generic_get_property_label(struct lxc_property_manager* mngr, char* property)
{
	struct lxc_property** props = mngr->properties;

	if(NULL == props)
		return NULL;

	for(int i=0;NULL != props[i];++i)
		if(0 == strcmp(property, props[i]->name))
			return props[i]->label;

	return NULL;
}

static const char* generic_get_property_description(struct lxc_property_manager* mngr, char* property)
{
	struct lxc_property** props = mngr->properties;

	if(NULL == props)
		return NULL;

	for(int i=0;NULL != props[i];++i)
		if(0 == strcmp(property, props[i]->name))
			return props[i]->description;

	return NULL;
}


static int generic_prop_operation
(
	Gate instance,
	struct lxc_property_manager* mngr,
	bool direction,
	char* property,
	char* value,
	char* dst,
	uint max_length
)
{
	struct lxc_property** props = mngr->properties;

	if(NULL == props)
		return LXC_ERROR_ENTITY_NOT_FOUND;

	for(int i=0;NULL != props[i];++i)
		if(0 == strcmp(property, props[i]->name))
		{
			struct lxc_property* prop = props[i];
			void* addr = mngr->access_property(instance, property);
			return prop->property_validator(instance, direction, addr, property, value, dst, max_length);
		}

	return LXC_ERROR_ENTITY_NOT_FOUND;
}

static inline struct lxc_property_manager* get_portb_propb_manager(Gate instance)
{
	struct lxc_generic_portb_propb_behavior* b =
		(struct lxc_generic_portb_propb_behavior*) instance->behavior;

	return &(b->properties);
}

static int generic_portb_propb_get_property_value(Gate instance, char* property, char* dst, uint max_length)
{
	return	generic_prop_operation
			(
				instance,
				get_portb_propb_manager(instance),
				DIRECTION_OUT,
				property,
				NULL,
				dst,
				max_length
			);
}

static int generic_portb_propb_set_property(Gate instance, char* property, char* value, char* err, uint max_length)
{
	return	generic_prop_operation
			(
				instance,
				get_portb_propb_manager(instance),
				DIRECTION_IN,
				property,
				value,
				err,
				max_length
			);
}

static int generic_portb_propb_enumerate_properties(Gate instance, const char** arr, uint max_index)
{
	return	generic_enumerate_properties
			(
				get_portb_propb_manager(instance),
				arr,
				max_index
			);
}


static const char* generic_portb_propb_get_property_label(Gate instance, char* property)
{
	return	generic_get_property_label
			(
				get_portb_propb_manager(instance),
				property
			);
}


static const char* generic_portb_propb_get_property_description(Gate instance, char* property)
{
	return	generic_get_property_description
			(
				get_portb_propb_manager(instance),
				property
			);
}

static inline struct lxc_property_manager* get_porti_propb_manager(Gate instance)
{
	struct lxc_generic_porti_propb_behavior* b =
		(struct lxc_generic_porti_propb_behavior*) instance->behavior;

	return &(b->properties);
}

static int generic_porti_propb_get_property_value(Gate instance, char* property, char* dst, uint max_length)
{
	return	generic_prop_operation
			(
				instance,
				get_porti_propb_manager(instance),
				DIRECTION_OUT,
				property,
				NULL,
				dst,
				max_length
			);
}

static int generic_porti_propb_set_property(Gate instance, char* property, char* value, char* err, uint max_length)
{
	return	generic_prop_operation
			(
				instance,
				get_porti_propb_manager(instance),
				DIRECTION_IN,
				property,
				value,
				err,
				max_length
			);
}

static int generic_porti_propb_enumerate_properties(Gate instance, const char** arr, uint max_index)
{
	return	generic_enumerate_properties
			(
				get_porti_propb_manager(instance),
				arr,
				max_index
			);
}


static const char* generic_porti_propb_get_property_label(Gate instance, char* property)
{
	return	generic_get_property_label
			(
				get_porti_propb_manager(instance),
				property
			);
}

static const char* generic_porti_propb_get_property_description(Gate instance, char* property)
{
	return	generic_get_property_description
			(
				get_porti_propb_manager(instance),
				property
			);
}

static void setup_default_properties
(
	struct lxc_property_manager* mngr,
	Gate gate
)
{
	if(NULL == mngr)
	{
		return;
	}

	if(NULL == mngr->properties)
	{
		return;
	}

	struct lxc_property** arr = mngr->properties;
	char error[200];
	for(int i=0;NULL != arr[i];++i)
	{
		lxc_set_property_value(gate, arr[i]->name, arr[i]->default_value, error, sizeof(error));
	}
}

static Gate lxc_generic_portb_propb_gate_create(const struct lxc_generic_portb_propb_behavior* behav)
{
	Gate ret = lxc_generic_portb_gate_create(behav);
	setup_default_properties(&(behav->properties), ret);
	return ret;
}

static Gate lxc_generic_porti_propb_gate_create(const struct lxc_generic_porti_propb_behavior* behav)
{
	Gate ret = lxc_generic_porti_gate_create(behav);
	setup_default_properties(&(behav->properties), ret);
	return ret;
}

struct lxc_generic_porti_propb_behavior lxc_generic_porti_propb_prototype;


void lxc_init_from_prototype(void* dst, size_t dst_len, void* src, size_t src_len)
{
	memcpy(dst, src, src_len);
	if(dst_len > src_len)
	{
		memset(offset_bytes(dst, dst_len), 0, dst_len-src_len);
	}
}

/*************************** Library Initialization ***************************/

void lxc_init_generic_library()
{
	/* create the "port and property manager in behavior"'s behavior
	 * from
	 * the basic "ports in behavior"'s behavior
	 */
	lxc_init_from_prototype
	(
		&lxc_generic_portb_propb_prototype,
		sizeof(lxc_generic_portb_propb_prototype),

		&lxc_generic_portb_prototype,
		sizeof(lxc_generic_portb_prototype)
	);

	struct lxc_gate_behavior* a = &(lxc_generic_portb_propb_prototype.base.base);

	lxc_generic_portb_propb_prototype.base.base.create =
		lxc_generic_portb_propb_gate_create;

	a->enumerate_properties = generic_portb_propb_enumerate_properties;
	a->get_property_label = generic_portb_propb_get_property_label;
	a->get_property_description = generic_portb_propb_get_property_description;
	a->get_property_value = generic_portb_propb_get_property_value;
	a->set_property = generic_portb_propb_set_property;


	lxc_init_from_prototype
	(
		&lxc_generic_porti_propb_prototype,
		sizeof(&lxc_generic_porti_propb_prototype),

		&lxc_generic_porti_prototype,
		sizeof(lxc_generic_porti_prototype)
	);

	struct lxc_gate_behavior* b = &(lxc_generic_porti_propb_prototype.base.base);

	lxc_generic_porti_propb_prototype.base.base.create =
		lxc_generic_porti_propb_gate_create;

	b->enumerate_properties = generic_porti_propb_enumerate_properties;
	b->get_property_label = generic_porti_propb_get_property_label;
	b->get_property_description = generic_porti_propb_get_property_description;
	b->get_property_value = generic_porti_propb_get_property_value;
	b->set_property = generic_porti_propb_set_property;

}

/************************ Generic Value implementations ***********************/

static void generic_value_free(LxcValue value)
{
	free(value);
}

LxcValue generic_value_clone(LxcValue dest)
{
	struct lxc_generic_value* val = (struct lxc_generic_value*) dest;
	struct lxc_generic_value* ret =
		(struct lxc_generic_value*) lxc_create_generic_value(val->base.type, val->size);
	memcpy(&(ret->data), &(val->data), val->size);
	return (LxcValue) ret;
}

size_t generic_value_size(LxcValue dest)
{
	return ((struct lxc_generic_value*) dest)->size;
}

static int generic_value_ref_diff(LxcValue value, int num)
{
	struct lxc_generic_value* val = (struct lxc_generic_value*) value;
	return __sync_fetch_and_add(&(val->refcount), num)+num;
}

void* generic_value_data_address(LxcValue value)
{
	return (((struct lxc_generic_value*)value)->data);
}

static struct lxc_value_operation lxc_generic_value_operations =
{
	.free = generic_value_free,
	.clone = generic_value_clone,
	.size = generic_value_size,
	.ref_diff = generic_value_ref_diff,
	.data_address = generic_value_data_address,
};

LxcValue lxc_create_generic_value(Signal type, size_t size)
{
	struct lxc_generic_value* ret =
		(struct lxc_generic_value*) malloc_zero(sizeof(struct lxc_generic_value)+size);
	ret->base.type = type;
	ret->base.operations = &lxc_generic_value_operations;
	ret->size = size;

	return (LxcValue)ret;
}
