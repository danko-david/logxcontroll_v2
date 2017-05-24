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
	if(NULL == type)
	{
		return NULL;
	}

	Wire ret = malloc_zero(sizeof(struct lxc_wire));
	ret->type = type;
	ret->subtype = 0;
	ret->current_value = NULL;

	ret->drivers = NULL;
	ret->drivers_length = 0;

	ret->drivens = NULL;
	ret->drivens_length = 0;

	return ret;
}

//TODO synchronize
bool remove_port(Gate instance, uint index, Tokenport* ports, int len)
{
	if(NULL == *ports)
	{
		return false;
	}
	else
	{
		int i = 0;
		while(i < len)
		{
			Tokenport p = ports[i];
			if(instance == p->gate && index == p->index)
			{
				array_nt_pop_element((void**)ports, len, i);
				//i found it
				free(p);
				return true;
			}

			++i;
		}

		return false;
	}
}


int private_generic_wiring
(
	Signal signal,
	int subtype,
	Wire wire,
	Gate instance,
	int index,
	int (*get_max_index)(Gate instance, Signal type, int subtype),
	int (*get_types)(Gate instance, Signal* arr, int* subs, uint max_length),
	void* (*get_wire)(Gate instance, Signal type, int subtype, uint index),
	bool is_subject_token,
	int (*wire_function)(Gate instance, Signal signal, int subtype, void* subject, uint index),
	void (*on_success)(Signal type, int subtype, void* subject, Gate g, uint index)
)
{
	//is the call legal?
	if(NULL == signal && NULL == wire)
	{
		return LXC_ERROR_BAD_CALL;
	}

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

	if(NULL == signal)
	{
		return LXC_ERROR_CORRUPTION;
	}

	//Check signal is supported
	Signal supp[LXC_GATE_MAX_IO_TYPE_COUNT];
	int sub[LXC_GATE_MAX_IO_TYPE_COUNT];

	int supp_max = get_types(instance, supp, sub, LXC_GATE_MAX_IO_TYPE_COUNT);

	if(supp_max < -LXC_GATE_MAX_IO_TYPE_COUNT)
		return LXC_ERROR_TOO_MANY_TYPES;

	int i = 0;

	while(i < supp_max)
	{
		if(supp[i] == signal && sub[i] == subtype)
			goto passed;

		++i;
	}

	return LXC_ERROR_TYPE_NOT_SUPPORTED;

passed:
	if(index < 0 || index > get_max_index(instance, signal, subtype))
	{
		return LXC_ERROR_ENTITY_OUT_OF_RANGE;
	}

	void* wired = get_wire(instance, signal, subtype, index);

	void* subject;

	//now user tries to unwire with the specified index
	if(NULL == wire)
	{
		//if already unwired
		if(NULL == wired)
		{
			return LXC_ERROR_PORT_IS_ALREADY_FREE;
		}

		//TODO
		//unwire the bound wire

		//unwiring is independent from the subject (Wire/token)
		wire_function(instance, signal, subtype, NULL, index);

		Tokenport* remove_from;
		uint remove_from_len;

		if(is_subject_token)
		{
			remove_from = ((Tokenport)wired)->owner->drivens;
			remove_from_len = ((Tokenport)wired)->owner->drivens_length;
		}
		else
		{
			remove_from = ((Wire)wired)->drivers;
			remove_from_len = ((Wire)wired)->drivers_length;
		}

		bool result = remove_port(instance, index, remove_from, remove_from_len);
		if(!result)
		{
			lxc_on_bug_found();
			return LXC_ERROR_NOT_CONNECTED_UNWIRING;
		}

		subject = NULL;
	}
	//tie a new wire in.
	else
	{
		if(NULL != wired)
		{
			return LXC_ERROR_PORT_IS_IN_USE;
		}

		//if subject is Tokenport we create and set a new one
		if(is_subject_token)
		{
			Tokenport p = malloc_zero(sizeof(struct lxc_tokenport));
			p->gate = instance;
			p->owner = wire;
			p->index = index;
			int ret = wire_function(instance, signal, subtype, p, index);
			if(0 != ret)
			{
				free(p);
				return ret;
			}
			subject = p;
		}
		else
		{
			int ret = wire_function(instance, signal, subtype, wire, index);
			if(0 != ret)
			{
				return ret;
			}
			subject = wire;
		}
	}

	on_success(signal, subtype, subject, instance, index);
	return 0;
}

//TODO synchronize
void add_port(Tokenport add, Tokenport** ports, uint* length)
{
	array_nt_append_element((void***) ports, length, (void*) add);
}

void on_successfull_input_wiring(Signal type, int subtype, void* /*Tokenport*/ tp, Gate g, uint index)
{
	//if successfully wired/unwired, register/deregister into the wire.
	//and notify the wire writer gate to re execute itself
	if(NULL != tp)
	{
		Tokenport p = (Tokenport) tp;
		add_port(p, &(p->owner->drivens), &(p->owner->drivens_length));
		p->wire_index = array_nt_contains((void**)p->owner->drivens, p->owner->drivens_length, (void*)p);
		if(g->enabled)
		{
			//TODO send a generic system notification
			//g->execution_behavior(g, type, wire->current_value, index);
		}
	}
	else
	{
		//this case is clear, notify unwired input
		if(lxc_gate_is_enabled(g))
		{
			g->execution_behavior(g, type, subtype, NULL, index);
		}
	}
}

void on_successfull_output_wiring(Signal type, int subtype, void* /*Wire*/ w, Gate g, uint index)
{
	Wire wire = (Wire) w;
	if(NULL != w)
	{
		//new wire attached, notify the driver
 		Tokenport p = malloc(sizeof(struct lxc_tokenport));
		p->gate = g;
		p->index = index;
		add_port(p, &(wire->drivers), &(wire->drivers_length));

		if(g->enabled)
		{
			LxcValue notify = lxc_create_system_event
			(
				system_event_output_wire_added,
				type,
				subtype,
				index,
				NULL
			);
			lxc_reference_value(notify);
			g->execution_behavior(g, notify->type, 0, notify, 0);
			lxc_unreference_value(notify);
		}
	}
	else
	{

		//notify driven gates, input is unwired
		/*
		if(lxc_gate_is_enabled(g))
		{
			g->execution_behavior(g, type, subtype, NULL, index);
		}

		 */
	}
}

int lxc_wire_gate_input(Signal type, int subtype, Wire wire, Gate g, uint index)
{
	//at this point type is maybe modified, but to a legal value.
	return private_generic_wiring
	(
		type,
		subtype,
		wire,
		g,
		index,
		g->behavior->get_input_max_index,
		g->behavior->get_input_types,
		(void*) g->behavior->get_input_wire,
		true,
		(void*) g->behavior->wire_input,
		on_successfull_input_wiring
	);
}

int lxc_wire_gate_output(Signal type, int subtype, Wire wire, Gate g, uint index)
{
	return private_generic_wiring
	(
		type,
		subtype,
		wire,
		g,
		index,
		g->behavior->get_output_max_index,
		g->behavior->get_output_types,
		(void*) g->behavior->get_output_wire,
		false,
		(void*) g->behavior->wire_output,
		on_successfull_output_wiring
	);
}

static void raise_wire_hook_calls
(
	enum lxc_wire_operation_phase phase,
	Wire this_wire,
	Gate subject_gate,
	uint subject_port_index,
	LxcValue value
)
{
	struct key_value** wire_debug_hooks = this_wire->wire_debug_hooks;
	if(NULL == wire_debug_hooks)
	{
		return;
	}

	int i;
	for(i=0;NULL != wire_debug_hooks[i];++i)
	{
		struct lxc_wire_debug_hook_data* data =
				(struct lxc_wire_debug_hook_data*) wire_debug_hooks[i]->value;

		//TODO reference and unreference the values (value, gate etc) before invoke

		data->wire_debug_hook
		(
			data,
			phase,
			this_wire,
			subject_gate,
			subject_port_index,
			value,
			this_wire->type,
			this_wire->subtype
		);
	}

}

/**
 * TODO for today: wire debugging hooks, and java callbacks for hooks
 *
 *
 * */
//TODO locks for wires
void lxc_drive_wire_value(Gate instance, uint out_index, Wire wire, LxcValue value)
{
	//TODO framework debugging mode, update wire value on change

	if(NULL == wire)
	{
		return;
	}

	struct key_value** wire_debug_hooks = wire->wire_debug_hooks;

	if(NULL != wire_debug_hooks)
	{
		raise_wire_hook_calls
		(
			lxc_before_wire_driven,
			wire,
			instance,
			out_index,
			value
		);
	}

	lxc_import_new_value(value, &(wire->current_value));

	/*Task t = malloc(sizeof(struct lxc_task));//TODO referenc the value beacuse it can change over the time, and can be finalized before another thread process it
	t->instance = instance;
	t->index = out_index;
	t->wire = wire;
	t->value = value;
	//TODO lxc_submit_asyncron_task(drive_wire_task_execute, t);
	//drive_wire_task_execute(t);

	//Gate instance = t->instance;
	//uint out_index = t->index;
	free(t);

	if(NULL != value)
	{
		lxc_unreference_value(value);
	}
*/
	Tokenport* ports = wire->drivens;
	Signal signal = wire->type;
	int len = wire->drivens_length;

	int i;
	for(i=0;i<len;++i)
	{
		Tokenport p = ports[i];
		if(NULL == p)
			return;

		if(p->gate->enabled && NULL != p->gate->execution_behavior)
		{
			if(NULL != wire_debug_hooks)
			{
				raise_wire_hook_calls
				(
					lxc_before_gate_notified,
					wire,
					p->gate,
					p->index,
					value
				);
			}

			p->gate->execution_behavior(p->gate, signal, wire->subtype, value, p->index);

			if(NULL != wire_debug_hooks)
			{
				raise_wire_hook_calls
				(
					lxc_after_gate_notified,
					wire,
					p->gate,
					p->index,
					value
				);
			}
		}

		if(NULL != wire_debug_hooks)
		{
			raise_wire_hook_calls
			(
				lxc_after_gate_notified,
				wire,
				p->gate,
				p->index,
				value
			);
		}

	}
}

int lxc_wire_add_debug_hook(Wire wire, struct lxc_wire_debug_hook_data* hook)
{
	if(NULL == wire || NULL == hook || NULL == hook->id || NULL == hook->wire_debug_hook)
	{
		return LXC_ERROR_BAD_CALL;
	}

	struct key_value** entries = (struct key_value**) wire->wire_debug_hooks;

	if(NULL != wire->wire_debug_hooks)
	{
		int i;
		for(i=0;NULL != entries[i];++i)
		{
			struct key_value* data = entries[i];
			if(0 == strcmp(hook->id, (const char*) data->key))
			{
				return LXC_ERROR_ENTITY_BY_NAME_ALREADY_REGISTERED;
			}
		}
	}

	struct key_value* data = malloc_zero(sizeof(struct key_value));
	data->key = (char*) hook->id;
	data->value = hook;
	array_pnt_append_element((void***)&wire->wire_debug_hooks, (void*) data);

	return 0;
}

struct lxc_wire_debug_hook_data* lxc_wire_remove_debug_hook(Wire wire, const char* id)
{
	if(NULL == wire || NULL == id)
	{
		return NULL;
	}

	struct key_value** entries = (struct key_value**) wire->wire_debug_hooks;

	if(NULL != entries)
	{
		int i;
		for(i=0;NULL != entries[i];++i)
		{
			struct key_value* data = entries[i];
			if(0 == strcmp(id, data->key))
			{
				return (struct lxc_wire_debug_hook_data*)
						array_pnt_pop_element((void***) &wire->wire_debug_hooks, i);
			}
		}

		return NULL;
	}
	else
	{
		return NULL;
	}
}

struct lxc_wire_debug_hook_data* lxc_wire_get_debug_hook(Wire wire, const char* id)
{
	struct key_value** entries = (struct key_value**) wire->wire_debug_hooks;
	if(NULL != entries)
	{
		int i;
		for(i=0;NULL != entries[i];++i)
		{
			struct key_value* data = entries[i];
			if(0 == strcmp(id, data->key))
			{
				return (struct lxc_wire_debug_hook_data*) data->value;
			}
		}
	}

	return NULL;
}

int lxc_reference_value(LxcValue value)
{
	return lxc_refdiff_value(value, 1);
}


int lxc_unreference_value(LxcValue value)
{
	return lxc_refdiff_value(value, -1);
}

int lxc_refdiff_value(LxcValue value, int count)
{
	if(NULL == value)
	{
		return 0;
	}

	int (*op)(LxcValue, int) = value->operations->ref_diff;
	if(NULL != op)
	{
		int ret = op(value, count);

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

	return 1024;
}

/* TODO
LxcValue lxc_get_wire_value(Wire w)
{
	if(NULL == w)
	{
		return NULL;
	}

	return w->current_value;
}
*/

LxcValue lxc_get_token_value(Tokenport tp)
{
	if(NULL == tp)
	{
		return NULL;
	}

	return tp->owner->current_value;

	//TODO atomic get
	struct chain_link_refc* lnk = tp->current_value;
	if(NULL == tp->current_value)
	{
		return NULL;
	}

	LxcValue val = (LxcValue) lnk->addr;
	return val;

}

//TODO
void lxc_absorb_token(Tokenport tp)
{

	//TODO atomic update next chain link
	//decremenet chainlink refcount
	//register "notify again" if port can notify
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

	return gate->behavior->gate_name;
}

bool lxc_gate_is_enabled(Gate gate)
{
	if(NULL == gate)
	{
		return true;
	}

	return gate->enabled;
}

void lxc_gate_set_enabled(Gate gate,bool enabled)
{
	if(NULL == gate)
	{
		return;
	}

	gate->enabled = enabled;

	void (*ivc)(Gate instance, Signal type, int subtype, LxcValue value, uint index) =
		gate->execution_behavior;

	if(NULL == ivc)
	{
		return;
	}

	LxcValue notify;
	if(enabled)
	{
		notify = lxc_create_system_event
		(
			system_event_gate_enabled,
			NULL,
			0,
			0,
			NULL
		);
	}
	else
	{
		notify = lxc_create_system_event
		(
			system_event_gate_disabled,
			NULL,
			0,
			0,
			NULL
		);
	}

	lxc_reference_value(notify);
	ivc(gate, notify->type, 0, notify, 0);
	lxc_unreference_value(notify);
}

int lxc_get_gate_input_types(Gate gate, Signal* sig, int* sub, int max_length)
{
	if(NULL == gate)
	{
		return 0;
	}

	//it's not mandatory to have inputs
	if(NULL != gate->behavior->get_input_types)
	{
		return gate->behavior->get_input_types(gate, sig, sub, max_length);
	}
	else
	{
		return 0;
	}
}


int lxc_get_gate_output_types(Gate gate, Signal* sig, int* sub, int max_length)
{
	if(NULL == gate)
	{
		return 0;
	}

	//it's not mandantory to have outputs
	if(NULL != gate->behavior->get_output_types)
	{
		return gate->behavior->get_output_types(gate, sig, sub, max_length);
	}
	else
	{
		return 0;
	}
}

static int get_io_min_max(bool direction, Gate gate, Signal s, int subtype)
{
	if(NULL == gate || NULL == s)
	{
		return 0;
	}

	int (*get_max)(Gate, Signal, int) =
		direction == DIRECTION_IN?
			gate->behavior->get_input_max_index
		:
			gate->behavior->get_output_max_index;

	if(NULL == get_max)
	{
		return 0;
	}

	return get_max(gate, s, subtype);
}


int lxc_get_gate_input_max_index(Gate gate, Signal s, int subtype)
{
	return get_io_min_max(DIRECTION_IN, gate, s, subtype);
}

int lxc_get_gate_output_max_index(Gate gate, Signal s, int subtype)
{
	return get_io_min_max(DIRECTION_OUT, gate, s, subtype);
}

static int fill_labels
(
	Gate gate,
	Signal type,
	int subtype,
	int max,
	const char* (*get_label)(Gate gate, Signal sig, int subtype, uint index),
	const char** arr,
	uint last_free,
	int max_length
)
{
	int permit = max_length - last_free;

	int i;
	for(i=0;i<max;++i)
	{
		const char* c = get_label(gate, type, subtype, i);
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
	int (*get_types)(Gate instance, Signal* arr, int* subs, uint max_length),
	int (*get_max_index)(Gate instance, Signal type, int subtype),
	const char* (*get_label)(Gate instance, Signal signal, int subtype, uint index)
)
{
	//assert has enough place for all port name
	Signal sigs[LXC_GATE_MAX_IO_TYPE_COUNT];
	int subs[LXC_GATE_MAX_IO_TYPE_COUNT];

	if(NULL == get_types)
	{
		return 0;
	}

	int len =	get_types
				(
					gate,
					sigs,
					subs,
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

	int i;
	for(i=0;i < len;++i)
	{
		int a;
		int req = get_max_index(gate, sigs[i], subs[i]);
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

	for(i=0;i < len;++i)
	{
		int req = get_max_index(gate, sigs[i], subs[i]);

		if(req <= 0)
		{
			continue;
		}

		sum += fill_labels
		(
			gate,
			sigs[i],
			subs[i],
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
	if(NULL == gate)
	{
		return 0;
	}

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
	if(NULL == gate)
	{
		return 0;
	}

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

static const char* get_io_label
(
	Gate gate,
	Signal type,
	int subtype,
	uint index,
	const char* (*get_label)(Gate instance, Signal signal, int subtype, uint index)
)
{
	if(NULL == type || NULL == get_label)
	{
		return NULL;
	}

	return get_label(gate, type, subtype, index);
}


const char* lxc_get_input_label(Gate gate, Signal type, int subtype, uint index)
{
	if(NULL == gate)
	{
		return NULL;
	}

	return	get_io_label
			(
				gate,
				type,
				subtype,
				index,
				gate->behavior->get_input_label
			);
}

const char* lxc_get_output_label(Gate gate, Signal type, int subtype, uint index)
{
	if(NULL == gate)
	{
		return NULL;
	}

	return	get_io_label
			(
				gate,
				type,
				subtype,
				index,
				gate->behavior->get_output_label
			);
}

const char* lxc_get_property_description(Gate gate, const char* property)
{
	if(NULL == gate || NULL == property)
	{
		return NULL;
	}

	if(NULL != gate->behavior->get_property_description)
	{
		return gate->behavior->get_property_description(gate, property);
	}

	return NULL;
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
	const char* value,
	char* error,
	uint max_len
)
{
	if(NULL == gate)
	{
		goto error;
	}

	int (*set_property)(Gate, const char*, const char*, char*, uint) =
		gate->behavior->set_property;

	if(NULL != set_property)
	{
		int ret = set_property(gate, property, value, error, max_len);

		if(0 == ret)
		{
			void (*ivc)(Gate instance, Signal type, int subtype, LxcValue value, uint index) =
					gate->execution_behavior;

			if(NULL != ivc)
			{
				LxcValue notify = lxc_create_system_event
				(
					system_event_property_modified,
					NULL,
					0,
					0,
					property
				);
				lxc_reference_value(notify);
				ivc(gate, notify->type, 0, notify, 0);
				lxc_unreference_value(notify);
			}
		}

		return ret;
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

Gate lxc_new_instance_by_behavior(const struct lxc_gate_behavior* b)
{
	if(NULL == b)
	{
		return NULL;
	}

	return b->create(b);
}

Gate lxc_new_instance_by_name(const char* name)
{
	const struct lxc_gate_behavior* behavior = get_gate_entry_by_name(name);
	if(NULL == behavior)
	{
		return NULL;
	}

	return behavior->create(behavior);
}


LxcValue lxc_get_constant_by_name(const char* name)
{
	if(NULL == REGISTERED_CONSTANT_VALUES)
		return NULL;

	int i;
	for(i=0;NULL != REGISTERED_CONSTANT_VALUES[i];++i)
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

	int i;
	for(i=0;NULL != REGISTERED_SIGNALS[i];++i)
	{
		if(0 == strcmp(str,REGISTERED_SIGNALS[i]->name))
			return REGISTERED_SIGNALS[i];
	}

	return NULL;
}


Tokenport lxc_get_input_wire(Gate gate, Signal signal, int subtype, uint index)
{
	if(NULL == gate)
	{
		return NULL;
	}

	return gate->behavior->get_input_wire(gate, signal, subtype, index);
}


Wire lxc_get_output_wire(Gate gate, Signal signal, int subtype, uint index)
{
	if(NULL == gate)
	{
		return NULL;
	}

	return gate->behavior->get_output_wire(gate, signal, subtype, index);
}

Signal lxc_get_signal_by_ordinal(int ordinal)
{
	if(ordinal < 0)
	{
		return NULL;
	}

	int len = array_pnt_population((void**) REGISTERED_SIGNALS);
	if(ordinal < len)
	{
		return NULL;
	}

	return REGISTERED_SIGNALS[ordinal];

}

