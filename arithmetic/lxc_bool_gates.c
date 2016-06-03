/*
 * lxc_bool_gates.c
 *
 *  Created on: 2016.01.22.
 *      Author: szupervigyor
 */

#include "lxc_bool_gates.h"
#include <stdlib.h>
#include <string.h>


static struct lxc_bool_gate_behavior* castBGB
(
	const struct lxc_gate_behavior* be
)
{
	return (struct lxc_bool_gate_behavior*) be;
}

static struct lxc_bool_instance* castGate(Gate gate)
{
	return (struct lxc_bool_instance*) gate;
}

static Gate logx_bool_create(const struct lxc_gate_behavior* be)
{
	struct lxc_bool_instance* ret = malloc_zero(sizeof(struct lxc_bool_instance));
	lxc_init_instance(&(ret->base), be);

	//memset(&(ret->inputs), 0, sizeof(ret->inputs));
	//ret->output = NULL;

	return &(ret->base);
}

static int logx_bool_get_supported_types
(
	Gate instance,
	Signal* arr,
	uint max_length
)
{
	UNUSED(instance);
	if(NULL == arr)
		return -1;

	if(max_length < 1)
		return -1;

	arr[0] = &lxc_signal_bool;

	if(max_length > 1)
		arr[1] = NULL;

	return 1;
}

static const char* const INPUT_NAMES[] =
{
	"I0",
	"I1",
	"I2",
	"I3",
	"I4",
	"I5",
	"I6",
	"I7",
	"I8",
	"I9",
	"I1",
	"I10",
	"I11",
	"I12",
	"I13",
	"I14",
	"I15",
	"I16",
	"I17",
	"I18",
	"I19",
	"I20",
};

static const char* const OUTPUT_NAMES[] =
{
	"Q0",
	"Q1",
	"Q2",
	"Q3",
	"Q4",
	"Q5",
	"Q6",
	"Q7",
	"Q8",
	"Q9",
	"Q1",
	"Q10",
	"Q11",
	"Q12",
	"Q13",
	"Q14",
	"Q15",
	"Q16",
	"Q17",
	"Q18",
	"Q19",
	"Q20",
};


static const char* logx_bool_get_input_label
(
	Gate instance,
	Signal signal,
	uint index
)
{
	UNUSED(instance);
	if(&lxc_signal_bool != signal || index > 20)
		return NULL;

	return INPUT_NAMES[index];
}

static const char* logx_bool_get_output_label(Gate instance, Signal signal,
		uint index)
{
	UNUSED(instance);
	if(&lxc_signal_bool != signal || index > 20)
		return NULL;

	return OUTPUT_NAMES[index];
}

static int return_20(Gate instance, Signal s)
{
	UNUSED(instance);
	if(&lxc_signal_bool != s)
			return -1;

	return 20;
}

static int return_1(Gate instance, Signal s)
{
	UNUSED(instance);
	if(&lxc_signal_bool != s)
		return -1;

	return 1;
}

static Wire logx_bool_get_input_wire(Gate instance, Signal signal, uint index)
{
	if(&lxc_signal_bool == signal || index > 20)
		return NULL;

	return castGate(instance)->inputs[index];
}

static Wire logx_bool_get_output_wire(Gate instance, Signal signal, uint index)
{
	if(&lxc_signal_bool == signal || 0 != index )
		return NULL;

	return castGate(instance)->output;
}

static int logx_bool_wire_input(Gate instance, Signal signal, Tokenport wire, uint index)
{
	UNUSED(instance);
	UNUSED(signal);
	castGate(instance)->inputs[index] = wire;
	return 0;
}

static int logx_bool_wire_output(Gate instance, Signal signal, Wire wire, uint index)
{
	UNUSED(instance);
	UNUSED(signal);
	UNUSED(index);
	castGate(instance)->output = wire;
	return 0;
}

static void logx_bool_execute(Gate instance, Signal type, LxcValue value, uint index)
{
	UNUSED(instance);
	UNUSED(type);
	UNUSED(index);
	UNUSED(value);
	Wire out = castGate(instance)->output;
	//if(NULL != out)
	{
		bool re = castBGB(instance->behavior)->logic_function(instance);

		lxc_drive_wire_value
		(
			instance,
			0,
			out,
			(LxcValue)
				(re?
					lxc_bool_constant_value_true.value
				:
					lxc_bool_constant_value_false.value
				)
		);
	}
}

/**
 * common function assigned for all logx bool gate.
 * */
static const struct lxc_bool_gate_behavior commons =
{
	.base.create = logx_bool_create,
	.base.destroy = lxc_destroy_simple_free,
	.base.get_input_types = logx_bool_get_supported_types,
	.base.get_input_label = logx_bool_get_input_label,
	.base.get_input_max_index = return_20,
	.base.get_input_wire = logx_bool_get_input_wire,
	.base.wire_input = logx_bool_wire_input,
	.base.execute = logx_bool_execute,
	.base.get_output_types = logx_bool_get_supported_types,
	.base.get_output_label = logx_bool_get_output_label,
	.base.get_output_max_index = return_1,
	.base.get_output_wire = logx_bool_get_output_wire,
	.base.wire_output = logx_bool_wire_output,
	.base.enumerate_properties = NULL,
	.base.get_property_label = NULL,
	.base.get_property_description = NULL,
	.base.get_property_value = NULL,
	.base.set_property = NULL,
};

#include <stdbool.h>

static bool is_all_input_valid_and_copy(Gate instance, bool values[21], int* ep)
{
	Tokenport* in = castGate(instance)->inputs;
	int to_absorb[21];
	int i = -1;
	*ep = 0;
	while(++i < 21)
	{
		Tokenport tp = in[i];

		//that means: unwired port
		if(NULL == tp)
		{
			continue;
		}

		LxcValue val = lxc_get_token_value(tp);

		if(NULL == val)
		{
			return false;
		}
		else
		{
			to_absorb[*ep] = i;
			values[*ep] = *((bool*)lxc_get_value(val));
			(*ep)++;
		}
	}

	i = -1;
	//absorb tokens
	while(++i < *ep)
	{
		lxc_absorb_token(in[to_absorb[i]]);
	}


	return true;
}

/************************** Logic Gate nand ***********************************/

/**
 * if there's no input wire, false returned.
 *
 * returns false if all input is true;
 * returns true if least one input is false
 *
 * */
static bool logic_function_nand(Gate instance)
{
	bool values[21];
	int ep;

	if(!is_all_input_valid_and_copy(instance, values, &ep))
	{
		return false;
	}

	int i = 0;
	while(i < ep)
	{
		if(0 == values[i])
		{
			return true;
		}
		++i;
	}

	return false;
}

static struct lxc_bool_gate_behavior logic_nand;


/************************** Logic Gate nor ************************************/

static bool logic_function_nor(Gate instance)
{
	bool values[21];
	int ep;

	if(!is_all_input_valid_and_copy(instance, values, &ep))
	{
		return false;
	}

	int i = 0;
	while(i < ep)
	{
		if(0 != values[i])
		{
			return false;
		}
		++i;
	}

	return 0 == ep?false:true;
}

static struct lxc_bool_gate_behavior logic_nor;

/************************** Logic Gate and ************************************/

static bool logic_function_and(Gate instance)
{
	bool values[21];
	int ep;

	if(!is_all_input_valid_and_copy(instance, values, &ep))
	{
		return false;
	}

	int i = 0;
	while(i < ep)
	{
		if(0 != values[i])
		{
			return false;
		}
		++i;
	}

	return 0 == ep?false:true;
}

static struct lxc_bool_gate_behavior logic_and;

/************************** Logic Gate or *************************************/

static bool logic_function_or(Gate instance)
{
	bool values[21];
	int ep;

	if(!is_all_input_valid_and_copy(instance, values, &ep))
	{
		return false;
	}

	int i = 0;
	while(i < ep)
	{
		if(0 != values[i])
		{
			return true;
		}
		++i;
	}

	return false;
}

static struct lxc_bool_gate_behavior logic_or;

/****************************** Only for debug ********************************/

static bool logic_function_dbg(Gate instance)
{
	Tokenport* in = castGate(instance)->inputs;

	int i=0;
	while(i<21)
	{
		Tokenport tp = in[i];
		if(NULL != tp)
		{
			LxcValue val = lxc_get_token_value(tp);
			if(NULL != val)
			{
				printf("Gate(dbg:%p) [%d]: %s\n", instance, i, *((char*)lxc_get_value(val))?"true":"false");
			}
		}

		++i;
	}


	return false;
}

static struct lxc_bool_gate_behavior logic_dbg;

/***************************** init functions *********************************/

static char*** path_to_primitive_logic =
	(char**[]){(char*[]){"Primitive", "Logic", NULL},NULL};

static void init_behavior
(
	struct lxc_bool_gate_behavior* b,
	const char* gate_name,
	bool(*logic_function)(Gate))
{
	memcpy(b, &commons, sizeof(commons));
	char** name = (char**)&(b->base.gate_name);
	*name = (char*)gate_name;
	//b->base.gate_name = gate_name;
	b->logic_function = logic_function;

	char**** path = &(b->base.paths);
	*path = path_to_primitive_logic;
}

static int library_operation_function(enum library_operation op, const char** errors, int max_length)
{
	if(library_before_load == op)
	{
		init_behavior(&logic_nand, "nand", logic_function_nand);
		init_behavior(&logic_nor, "nor", logic_function_nor);
		init_behavior(&logic_and, "and", logic_function_and);
		init_behavior(&logic_or, "or", logic_function_or);
		init_behavior(&logic_dbg, "dbg", logic_function_dbg);
		return 0;
	}

	/*else if(library_after_unloaded == op)
	{
		return 0;
	}*/

	return 0;
}

//loadable library definition
#ifdef LXC_EMBED_MODULE_ARITHMETIC
	struct lxc_loadable_library logxcontroll_loadable_library_arithmetic
#else
	struct lxc_loadable_library logxcontroll_loadable_library
#endif
=
{
	.library_operation = library_operation_function,
	.gates = (struct lxc_gate_behavior*[])
	{
		&logic_nand,
		&logic_nor,
		&logic_and,
		&logic_or,

		&logic_dbg,

		NULL,
	},


};
