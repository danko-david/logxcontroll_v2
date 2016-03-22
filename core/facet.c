/*
 * debug.c
 *
 *  Created on: 2016.03.22.
 *      Author: szupervigyor
 */

#include "core/logxcontroll.h"

/**
 * Wiring function:
 * 	wire_input and wire_output used for binding wires into/from the gate or
 * 	unwire a specified port
 *
 * example code:
 *
 	int wiring(Gate instance, Signal signal, Wire wire, int index)
 	{
 		//is the call legal?
 		if(NULL == signal && NULL == wire)
 			return WIRING_ERROR_BAD_CALL;

		if(NULL == signal)
		{
			signal = wire->type;
		}
		//signal is not null. But if the wire not null it is use the same type?
		else if(NULL != wire && signal != wire->type)
		{
			return WIRING_ERROR_BAD_CALL;
		}

		if(!is_im_support_this_type_of_signal(signal))
		{
			return WIRING_ERROR_TYPE_NOT_SUPPORTED;
		}

		if(!is_wire_by_type__index_is_in_range(signal, index))
		{
			return WIRING_ERROR_PORT_OUT_OF_RANGE;
		}

		//now user tries to unwire with specified index
		if(NULL == wire)
		{
			if(is_target_wire_NULL(signal, index))
			{
				return WIRING_ERROR_PORT_IS_ALREADY_FREE;
			}

			set_target_wire(signal, index, NULL);
			return 0;
		}
		else
		{
			if(!is_target_wire_NULL(signal, index))
			{
				return WIRING_ERROR_PORT_IS_IN_USE;
			}

			set_target_wire(signal, index, wire);
			return 0;
		}
	}
 *
 * */


/**
 * destionation arrays:
 * 	functions prototyped like: int function(array** arr, int max_length);
 *	tries to copy the data to the given destination address.
 *
 *	some examples:
 *
 *	every time the item count is 5.
 *
 *	{
 *		struct item* arr[1];
 *		int ret = function(arr, 1);
 *		//ret = -5
 *		//no modification in arr memory region
 *	}
 *
 *	{
 *		struct item* arr[5];
 *		int ret = function(arr, 5);
 *		//ret = 5
 *		//all the data are copied.
 *	}
 *
 *	{
 *		struct item* arr[10];
 *		int ret = function(arr, 10);
 *		//ret = 5
 *		//all data copied and arr is terminated. (arr[6] = NULL)
 *	}
 *
 *	{
 *		int ret = function(NULL, 0);
 *		//ret = -5
 *		//get the size
 *	}
 *
 * */


Wire lxc_create_wire(Signal type)
{
	Wire ret = malloc(sizeof(struct lxc_wire));
	ret->type = type;
	ret->current_value = NULL;

	ret->drivers = NULL;
	ret->drivers_length = 0;

	ret->drivens = NULL;
	ret->drivens_length = 0;

	return ret;
}

//TODO synchronize
int remove_port(Gate instance, uint index, Port** ports, uint* length)
{
	if(NULL == *ports)
	{
		return false;
	}
	else
	{
		int len = *length;
		int i = 0;
		while(i < len)
		{
			Port p = (*ports)[i];
			if(instance == p->gate && index == p->index)
			{
				//i found them
				free(p);

				//shifting the array if least one element on the upper index
				--len;
				//len is now lower, no extra len+1 needed in the next ops.
				if(i < len)
				{
					while(i < len)
					{
						(*ports)[i] = (*ports)[i+1];
						++i;
					}
				}

				//set the last element NULL
				//(no duplication at the end of the array)
				(*ports)[len] = NULL;
				return 1;
			}

			++i;
		}


		return 0;
	}
}


int private_generic_wiring
(
	Signal signal,
	Wire wire,
	Gate instance,
	int index,
	int (*get_max_index)(Gate instance, Signal type),
	int (*get_types)(Gate instance, Signal* arr, uint max_length),
	Wire (*get_wire)(Gate instance, Signal type, uint index),
	int (*wire_function)(Gate instance, Signal signal, Wire wire, uint index),
	void (*on_success)(Signal type, Wire wire, Gate g, uint index)
)
{

	//is the call legal?
	if(NULL == signal && NULL == wire)
		return LXC_ERROR_BAD_CALL;

	//setting value if missing
	if(NULL == signal)
	{
		signal = wire->type;
	}
	//signal is not null. But if the wire not null it is use the same type?
	else if(NULL != wire && signal != wire->type)
	{
		return LXC_ERROR_BAD_CALL;
	}

	//Check signal is supported
	Signal supp[LXC_GATE_MAX_IO_TYPE_COUNT];

	int supp_max = get_types(instance, supp, LXC_GATE_MAX_IO_TYPE_COUNT);

	if(supp_max < -LXC_GATE_MAX_IO_TYPE_COUNT)
		return LXC_ERROR_TOO_MANY_TYPES;

	int i = 0;

	while(i < supp_max)
	{
		if(supp[i] == signal)
			goto passed;

		++i;
	}

	return LXC_ERROR_TYPE_NOT_SUPPORTED;

passed:


	if(index < 0 || index > get_max_index(instance, signal))
	{
		return LXC_ERROR_PORT_OUT_OF_RANGE;
	}

	Wire wired = get_wire(instance, signal, index);

	//now user tries to unwire with specified index
	if(NULL == wire)
	{
		if(NULL == wired)
		{
			return LXC_ERROR_PORT_IS_ALREADY_FREE;
		}

		wire_function(instance, signal, NULL, index);
	}
	else
	{
		if(NULL != wired)
		{
			return LXC_ERROR_PORT_IS_IN_USE;
		}

		wire_function(instance, signal, wire, index);
	}

	//int ret = private_check_signal(g, &type, wire, g->behavior->get_input_types);

	//mi van ha a vezetéken rajta van de ő azt mondja hogy nincs?
	//vagy ha már a vezetéken rajta van de ő azt mondja nem?

	//unwire gate
	if(NULL == wire)
	{
		//unwire the input.
		bool result = remove_port(instance, index, &(wire->drivens), &(wire->drivens_length));
		if(!result)
		{
			return LXC_ERROR_NOT_CONNECTED_UNWIRING;
		}
	}

	on_success(signal, wire, instance, index);
	return 0;
}

//TODO synchronize
void add_port(Port add, Port** ports, uint* length)
{
	array_nt_append_element((void***)ports, length, add);
}

void on_successfull_input_wiring(Signal type, Wire wire, Gate g, uint index)
{
	//if successfully wired/unwired, register/deregister into the wire.
	//and notify the wire writer gate to re execute itself
	if(NULL != wire)
	{
		Port p = malloc(sizeof(struct lxc_port));
		p->gate = g;
		p->index = index;
		add_port(p, &(wire->drivens), &(wire->drivens_length));

		//TODO how to notify in this case? the driver or the driven gate?
		//notify_drivers(wire);

		//TODO in this case reference count management
		if(g->enabled)
			g->behavior->input_value_changed(g, type, wire->current_value, index);
	}
	else
	{
		//this case is clear, notify unwired input
		g->behavior->input_value_changed(g, type, NULL, index);
	}
}

void on_successfull_output_wiring(Signal type, Wire wire, Gate g, uint index)
{
	if(NULL != wire)
	{
		//new wire attached, send to the driver a general input_change
		Port p = malloc(sizeof(struct lxc_port));
		p->gate = g;
		p->index = index;
		add_port(p, &(wire->drivers), &(wire->drivers_length));

		if(g->enabled)
			g->behavior->input_value_changed(g, NULL, NULL, 0);
	}
	else
	{
		//notify driven gates, input is unwired
		Port* ps = wire->drivens;
		int len = wire->drivens_length;
		int i = 0;
		while(i<len)
		{
			Port p = ps[i];
			if(NULL == p)
				return;

			Gate target = p->gate;
			if(target->enabled)
				target->behavior->input_value_changed(target, type, NULL, p->index);
		}
	}
}

int lxc_wire_gate_input(Signal type, Wire wire, Gate g, uint index)
{
	//at this point type is maybe modified, but to a legal value.
	return private_generic_wiring
	(
		type,
		wire,
		g,
		index,
		g->behavior->get_input_max_index,
		g->behavior->get_input_types,
		g->behavior->get_input_wire,
		g->behavior->wire_input,
		on_successfull_input_wiring
	);
}

int lxc_wire_gate_output(Signal type, Wire wire, Gate g, uint index)
{
	return private_generic_wiring
	(
		type,
		wire,
		g,
		index,
		g->behavior->get_output_max_index,
		g->behavior->get_output_types,
		g->behavior->get_output_wire,
		g->behavior->wire_output,
		on_successfull_output_wiring
	);
}


//TODO locks for wires
void lxc_drive_wire_value(Gate instance, uint out_index, Wire wire, LxcValue value)
{
	UNUSED(instance);
	UNUSED(out_index);

	//TODO framework debugging mode, update wire value on change

	//tryfree old value

	int i = 0;
	Port* ports = wire->drivens;
	Signal signal = wire->type;
	int len = wire->drivens_length;

	//TODO reference value
	wire->current_value = value;
	while(i < len)
	{
		Port p = ports[i];
		if(NULL == p)
			return;

		if(p->gate->enabled)
			p->gate->behavior->input_value_changed(p->gate, signal, value, p->index);

		++i;
	}
	//TODO unreferece value and free if no more ref.
}


int lxc_reference_value(LxcValue value)
{
	if(NULL == value)
	{
		return 0;
	}

	int (*op)(LxcValue, int) = value->operations->ref_diff;
	if(NULL != op)
	{
		return op(value, 1);
	}

	//not a reference counted type
	return 1;
}


int lxc_unreference_value(LxcValue value)
{
	if(NULL == value)
	{
		return 0;
	}

	int (*op)(LxcValue, int) = value->operations->ref_diff;
	if(NULL != op)
	{
		int ret = op(value, 1);

#ifdef DEBUG_FOR_NEGATIVE_REFCOUNT
		if(ret < 0)
		{
			char str[200];
			gnu_libc_backtrace_symbol(value, str, sizeof(str));

			printf("WARNING: negative reference count for type: \"%s\", value: %s", value->type->name, str);
		}
#endif
		if(ret <= 0)
		{
			void (*ff)(LxcValue) = value->operations->free;
			if(NULL != ff)
			{
				ff(value);
			}
		}

		return ret;
	}


	return 1;
}


LxcValue lxc_get_wire_value(Wire w)
{
	return w->current_value;
}


void* lxc_get_value(LxcValue v)
{
	if(NULL == v)
		return NULL;

	return v->operations->data_address(v);
}


const char* lxc_get_gate_name(Gate gate)
{
	if(NULL == gate)
	{
		return NULL;
	}

	return gate->behavior->get_gate_name(gate);
}


int lxc_get_gate_input_types(Gate gate, Signal* sig, int max_length)
{
	if(NULL == gate)
	{
		return 0;
	}

	//it's not mandantory to have inputs
	if(NULL != gate->behavior->get_input_types)
	{
		return gate->behavior->get_input_types(gate, sig, max_length);
	}
	else
	{
		return 0;
	}
}


int lxc_get_gate_output_types(Gate gate, Signal* sig, int max_length)
{
	if(NULL == gate)
	{
		return 0;
	}

	//it's not mandantory to have outputs
	if(NULL != gate->behavior->get_output_types)
	{
		return gate->behavior->get_output_types(gate, sig, max_length);
	}
	else
	{
		return 0;
	}
}


static int fill_labels
(
	Gate gate,
	Signal type,
	int max,
	const char* (*get_label)(Gate gate, Signal sig, uint index),
	const char** arr,
	uint last_free,
	int max_length
)
{
	int permit = max_length - last_free;
	for(int i=0;i<max;++i)
	{
		const char* c = get_label(gate, type, i);
		arr[last_free+i] = c;

		if(--permit == 0)
			return i;
	}

	return max;
}


static int get_labels
(
	Gate gate,
	const char** arr,
	int max_length,
	int (*get_types)(Gate instance, Signal* arr, uint max_length),
	int (*get_max_index)(Gate instance, Signal type),
	const char* (*get_label)(Gate instance, Signal signal, uint index)
)
{
	if(NULL == gate)
		{
			return 0;
		}

		//assert has enough place for all port name
		Signal sigs[LXC_GATE_MAX_IO_TYPE_COUNT];

		if(NULL == get_types)
		{
			return 0;
		}

		int len =	get_types
					(
						gate,
						sigs,
						LXC_GATE_MAX_IO_TYPE_COUNT
					);

		if(0 == len)
		{
			return 0;
		}
		else if(len < 0)
		{
			return LXC_ERROR_TOO_MANY_TYPES;
		}

		int req_length = 0;

		for(int i=0;i < len;++i)
		{
			int req = get_max_index(gate, sigs[i]);
			if(req > 0)
			{
				req_length += req;
			}
		}

		if(max_length <= req_length)
		{
			return -req_length;
		}

		int sum = 0;

		for(int i=0;i < len;++i)
		{
			int req = get_max_index(gate, sigs[i]);

			if(req <= 0)
			{
				continue;
			}

			sum += fill_labels
			(
				gate,
				sigs[i],
				req,
				get_label,
				arr,
				sum,
				max_length
			);
		}

		return sum;
}


int lxc_get_input_labels(Gate gate, const char** arr, int max_length)
{
	return get_labels
	(
		gate,
		arr,
		max_length,
		gate->behavior->get_input_types,
		gate->behavior->get_input_max_index,
		gate->behavior->get_input_label
	);
}


int lxc_get_output_labels(Gate gate, const char** arr, int max_length)
{
	return get_labels
	(
		gate,
		arr,
		max_length,
		gate->behavior->get_output_types,
		gate->behavior->get_output_max_index,
		gate->behavior->get_output_label
	);
}


int lxc_enumerate_properties(Gate gate, const char** arr, int max_length)
{
	if(NULL == gate)
	{
		return 0;
	}

	int (*ep)(Gate instance, const char** arr, uint max_length) =
		gate->behavior->enumerate_properties;

	if(NULL == ep)
	{
		return 0;
	}
	else
	{
		return ep(gate, arr, max_length);
	}
}

int lxc_set_property_value
(
	Gate gate,
	const char* property,
	char* value,
	char* error,
	uint max_len
)
{
	if(NULL == gate)
	{
		goto error;
	}

	int (*set_property)(Gate, char*, char*, char*, uint) =
		gate->behavior->set_property;

	if(NULL != set_property)
	{
		return set_property(gate, property, value, error, max_len);
	}

error:

	safe_strcpy(error, max_len, "");
	return -1;
}

const char* lxc_get_property_label(Gate gate, const char* prop)
{
	if(NULL == gate)
	{
		return NULL;
	}

	const char* (*pl)(Gate instance, const char* property) =
		gate->behavior->get_property_label;

	if(NULL == pl)
	{
		return NULL;
	}

	return pl(gate, prop);
}

int lxc_get_property_value(Gate gate, const char* prop, char* ret, int max)
{
	if(NULL == gate)
	{
		safe_strcpy(ret, max, "");
		return -1;
	}

	int (*pv)(Gate, const char*, char*, uint) =
		gate->behavior->get_property_value;

	if(NULL == pv)
	{
		safe_strcpy(ret, max, "");
		return -1;
	}

	return pv(gate, prop, ret, max);
}


Gate lxc_new_instance_by_name(const char* name)
{
	struct detailed_gate_entry* ent = get_gate_entry_by_name(name);
	if(NULL == ent)
		return NULL;

	const struct lxc_gate_behavior* behavior = ent->behavior;
	if(NULL == behavior)
		return NULL;

	return behavior->create(behavior);
}


LxcValue lxc_get_constant_by_name(char* name)
{
	if(NULL == REGISTERED_CONSTANT_VALUES)
		return NULL;

	for(int i=0;NULL != REGISTERED_CONSTANT_VALUES[i];++i)
	{
		if(0 == strcmp(name,REGISTERED_CONSTANT_VALUES[i]->name))
			return REGISTERED_CONSTANT_VALUES[i]->value;
	}

	return NULL;
}


Signal lxc_get_signal_by_name(const char* str)
{
	if(NULL == REGISTERED_SIGNALS)
		return NULL;

	for(int i=0;NULL != REGISTERED_SIGNALS[i];++i)
	{
		if(0 == strcmp(str,REGISTERED_SIGNALS[i]->name))
			return REGISTERED_SIGNALS[i];
	}

	return NULL;
}
