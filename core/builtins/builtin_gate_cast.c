/*
 * builtin_gate_cast.c
 *
 *  Created on: 2016.02.27.
 *      Author: szupervigyor
 */


#include "core/logxcontroll.h"

struct lxc_cast_instance
{
	struct lxc_generic_porti_instance base;
	Wire input;
	Wire output;
	Signal from;
	Signal to;
	LxcValue (*cast_to)(LxcValue);
};

static struct lxc_generic_porti_propb_behavior gate_cast;

struct detailed_gate_entry lxc_built_in_gate_cast =
{
	.behavior = &(gate_cast.base),
	.generic_name = "cast to",
};

static void* cast_access_property(Gate instance, const char* property)
{
	struct lxc_cast_instance* gate = (struct lxc_cast_instance*) instance;
	if(0 == strcmp("from", property))
	{
		return &(gate->from);
	}
	else if(0 == strcmp("to", property))
	{
		return &(gate->to);
	}

	return NULL;
}

static void replace_portmanager_type
(
	struct lxc_port_manager* mngr,
	Signal new_type,
	char* name
)
{
	int absindex = lxc_port_get_absindex_by_name(mngr, name);

	if(absindex >= 0)
	{
		Signal sig;
		int mti;
		int index;
		lxc_port_get_type_and_index_by_absindex(mngr, absindex, &sig, &mti, &index);
		if(NULL != sig)
		{
			lxc_port_remove_port(mngr, sig, index);
		}
		else
		{
			printf("ERROR: built in gate cast: registered type not found.");
			return;
		}
	}

	lxc_port_unchecked_add_new_port(mngr, name, new_type, true);
}


static void replace_port_type
(
	struct lxc_cast_instance* gate,
	void* addr,
	Signal new_type
)
{
	if(addr == &(gate->from))
	{
		replace_portmanager_type(&(gate->base.input_ports), new_type, "from");
	}
	else if(addr == &(gate->to))
	{
		replace_portmanager_type(&(gate->base.output_ports), new_type, "to");
	}
}

static int cast_validate_propety
(
	struct lxc_cast_instance* gate,
	Wire wire,
	bool direction,
	void* addr,
	const char* name,
	const char* value,
	char* ret,
	int max_length
)
{
	if(direction == DIRECTION_OUT)
	{
		//write back which type of signal is currently used for input type
		if(NULL == addr)
		{
			safe_strcpy(ret, max_length, "");
			return 0;
		}

		if(NULL == *((Signal*)addr))
		{
			return safe_strcpy(ret, max_length, "");
		}

		safe_strcpy(ret, max_length, (char*) (*((Signal*)addr))->name);
		return 0;
	}
	else if(direction == DIRECTION_IN)
	{
		//if port is wired we refuse the signal modification
		if(NULL != wire)
		{
			safe_strcpy(ret, max_length, "");
			return LXC_ERROR_PORT_IS_IN_USE;
		}

		Signal sig = lxc_get_signal_by_name(value);

		if(NULL == sig)
		{
			safe_strcpy(ret, max_length, "Requested signal type doesn't exists.");
			return LXC_ERROR_TYPE_NOT_SUPPORTED;
		}

		Signal t_from = NULL;
		Signal t_to = NULL;

		//is there a know direct conversion between the requested types?
		if(addr == &(gate->from))
		{
			t_from = sig;

			if(NULL != gate->to)
				t_to = gate->to;
		}
		else if(addr == &(gate->to))
		{
			t_to = gate->to;
			if(NULL != gate->from)
				t_from = gate->from;
		}

		if(NULL != t_from && NULL != t_to)
		{
			LxcValue (*convert)(LxcValue) =
				lxc_get_conversion_function(t_from, t_to);

			if(NULL == convert)
			{
				safe_strcpy(ret, max_length, "No direct conversion known between specified types.");
				return LXC_ERROR_TYPE_CONVERSION_NOT_EXISTS;
			}
			else
			{
				replace_port_type(gate, addr, sig);
				*((Signal*)addr) = sig;
				gate->cast_to = convert;
				return 0;
			}
		}
		else
		{
			replace_port_type(gate, addr, sig);
			*((Signal*)addr) = sig;
		}

		return 0;
	}

	//may not happen.
	return LXC_ERROR_ILLEGAL_REQUEST;
}


static int validate_property_from
(
	Gate instance,
	bool direction,
	void* addr,
	const char* name,
	const char* value,
	char* ret,
	int max_length
)
{
	struct lxc_cast_instance* gate = (struct lxc_cast_instance*) instance;
	return cast_validate_propety(gate, gate->input, direction, addr, name, value, ret, max_length);
}

static int validate_property_to
(
	Gate instance,
	bool direction,
	void* addr,
	const char* name,
	const char* value,
	char* ret,
	int max_length
)
{
	struct lxc_cast_instance* gate = (struct lxc_cast_instance*) instance;
	return cast_validate_propety(gate, gate->output, direction, addr, name, value, ret, max_length);
}

static void cast_execute(Gate instance, Signal type, LxcValue value, uint index)
{


}

void lxc_builtin_cast_init_before_load()
{
	memset(&gate_cast, 0, sizeof(gate_cast));

	memcpy
	(
		&gate_cast,
		&lxc_generic_porti_propb_prototype,
		sizeof(lxc_generic_porti_propb_prototype)
	);

	gate_cast.base.instance_memory_size = sizeof(struct lxc_cast_instance);
	lxc_built_in_gate_cast.paths = lxc_built_in_path_type,

	//yes we discard const
	//void* (**ap_addr)(Gate, const char*) = &();

	gate_cast.properties.access_property = cast_access_property;
	gate_cast.base.base.execute = cast_execute;
	gate_cast.base.gate_name = "cast to";

	//*ap_addr = ;

	lxc_add_property
	(
		&(gate_cast.properties),
		"from",
		"From type",
		"Cast value type from",
		"",
		validate_property_from
	);

	lxc_add_property
	(
		&(gate_cast.properties),
		"to",
		"To type",
		"Cast value type to",
		"",
		validate_property_to
	);
}




