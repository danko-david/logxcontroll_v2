/*
 * lxc_bool_gates.c
 *
 *  Created on: 2016.01.22.
 *      Author: szupervigyor
 */

#include "core/builtins/logic/lxc_bool_gates.h"
#include <stdlib.h>
#include <string.h>


static struct lxc_bool_gate_behavior* castBGB(const struct lxc_gate_behavior* be)
{
	return (struct lxc_bool_gate_behavior*) be;
}

static struct lxc_bool_instance* castGate(Gate gate)
{
	return (struct lxc_bool_instance*) gate;
}

static const char* logx_bool_get_gate_name(Gate gate)
{
	return castBGB(gate->behavior)->name;
}

static Gate logx_bool_create(const struct lxc_gate_behavior* be)
{
	struct lxc_bool_instance* ret = malloc(sizeof(struct lxc_bool_instance));
	lxc_init_instance(&(ret->base), be);

	memset(&(ret->inputs), 0, sizeof(ret->inputs));

	ret->output = NULL;

	return &(ret->base);
}

static int logx_bool_get_supported_types(Gate instance, Signal* arr, uint max_length)
{
	UNUSED(instance);
	if(NULL == arr)
		return -1;

	if(max_length < 1)
		return -1;

	arr[0] = type_bool;

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


static const char* logx_bool_get_input_label(Gate instance, Signal signal, uint index)
{
	UNUSED(instance);
	if(type_bool != signal || index > 20)
		return NULL;

	return INPUT_NAMES[index];
}

static const char* logx_bool_get_output_label(Gate instance, Signal signal,
		uint index)
{
	UNUSED(instance);
	if(type_bool != signal || index > 20)
		return NULL;

	return OUTPUT_NAMES[index];
}

static const char* logx_bool_get_single_ouput_label(Gate instance, Signal signal,
		int index)
{
	UNUSED(instance);
	if(type_bool != signal || 0 != index)
		return NULL;

	return "Q";
}

static int return_20(Gate instance, Signal s)
{
	UNUSED(instance);
	if(type_bool != s)
			return -1;

	return 20;
}

static int return_1(Gate instance, Signal s)
{
	UNUSED(instance);
	if(type_bool != s)
		return -1;

	return 1;
}

static Wire logx_bool_get_input_wire(Gate instance, Signal signal, uint index)
{
	if(type_bool == signal || index > 20)
		return NULL;

	return castGate(instance)->inputs[index];
}

static Wire logx_bool_get_output_wire(Gate instance, Signal signal, uint index)
{
	if(type_bool == signal || 0 != index )
		return NULL;

	return castGate(instance)->output;
}

static int logx_bool_wire_input(Gate instance, Signal signal, Wire wire, uint index)
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
	if(NULL != out)
	{
		bool re = castBGB(instance->behavior)->logic_function(instance);
		lxc_drive_wire_value(instance, 0, out, (LxcValue) (re? bool_value_true: bool_value_false));
	}
}

static void logx_bool_input_value_changed(Gate instance, Signal type, LxcValue value, uint index)
{
	instance->execution_behavior(instance, type, value, index);
}

/**
 * common function assigned for all logx bool gate.
 * */


static const struct lxc_bool_gate_behavior commons =
{
	.base.get_gate_name = logx_bool_get_gate_name,
	.base.create = logx_bool_create,
	.base.destroy = lxc_destroy_simple_free,
	.base.get_input_types = logx_bool_get_supported_types,
	.base.get_input_label = logx_bool_get_input_label,
	.base.get_input_max_index = return_20,
	.base.get_input_wire = logx_bool_get_input_wire,
	.base.wire_input = logx_bool_wire_input,
	.base.input_value_changed = logx_bool_input_value_changed,
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
	.base.gatectl = NULL,
	.name = NULL,
	.logic_function = NULL,
};

#include <stdbool.h>

/************************** Logic Gate nand ***********************************/

static bool logic_function_nand(Gate instance)
{
	Wire* in = castGate(instance)->inputs;
	int i = 0;
	while(i < 21)
	{
		Wire w = in[i];
		if(NULL == w)
			continue;

		LxcValue val = lxc_get_wire_value(w);
		if(NULL != val)
		{
			char val = *((char*)lxc_get_value(val));
			if(0 != val)
				return true;
		}
		++i;
	}

	return false;
}

static struct lxc_bool_gate_behavior logic_nand;
static struct detailed_gate_entry detail_logic_nand =
{
	.behavior = &(logic_nand.base),
	.generic_name = "nand",
	.paths = (char**[]){(char*[]){"Primitive", "Logic", NULL},NULL},

};

/************************** Logic Gate nor ************************************/

static bool logic_function_nor(Gate instance)
{
	Wire* in = castGate(instance)->inputs;
	int i = 0;
	int used = 0;
	while(i < 21)
	{
		Wire w = in[i];
		if(NULL == w)
			continue;

		++used;
		LxcValue val = lxc_get_wire_value(w);
		if(NULL != val)
		{
			char val = *((char*)lxc_get_value(val));
			if(0 != val)
				return false;
		}
		++i;
	}

	return 0 == used?false:true;
}

static struct lxc_bool_gate_behavior logic_nor;
static struct detailed_gate_entry detail_logic_nor =
{
	.behavior = &logic_nor,
	.generic_name = "nor",
	.paths = (char**[]){(char*[]){"Primitive", "Logic", NULL},NULL},
};

/************************** Logic Gate and ************************************/

static bool logic_function_and(Gate instance)
{
	Wire* in = castGate(instance)->inputs;
	int i = 0;
	int used = 0;
	while(i < 21)
	{
		Wire w = in[i];
		if(NULL == w)
			continue;

		++used;
		LxcValue val = lxc_get_wire_value(w);
		if(NULL != val)
		{
			char val = *((char*)lxc_get_value(val));
			if(0 == val)
				return false;
		}
		++i;
	}

	return 0 == used?false:true;
}

static struct lxc_bool_gate_behavior logic_and;
static struct detailed_gate_entry detail_logic_and =
{
	.behavior = &logic_and,
	.generic_name = "and",
	.paths = (char**[]){(char*[]){"Primitive", "Logic", NULL},NULL},
};

/************************** Logic Gate or *************************************/

static bool logic_function_or(Gate instance)
{
	Wire* in = castGate(instance)->inputs;
	int i = 0;
	int used = 0;
	while(i < 21)
	{
		Wire w = in[i];
		if(NULL == w)
			continue;

		++used;
		LxcValue val = lxc_get_wire_value(w);
		if(NULL != val)
		{
			char val = *((char*)lxc_get_value(val));
			if(0 != val)
				return true;
		}
		++i;
	}

	return false;
}

static struct lxc_bool_gate_behavior logic_or;
static struct detailed_gate_entry detail_logic_or =
{
	.behavior = &logic_or,
	.generic_name = "or",
	.paths = (char**[]){(char*[]){"Primitive", "Logic", NULL},NULL},
};

/****************************** Only for debug ********************************/

/*void dbg_print_exec(Gate instance, Signal type, LxcValue value, uint index)
{
	printf
	(
		"Gate: %p, Signal: %s, index: %d, value: %s\n",
		instance,
		NULL == type?"NULL":type->name,
		index,
		NULL == value?"NULL":
			(((struct lxc_primitive_value*)value)->char_value?"true":"false")
	);
}


struct lxc_bool_gate_behavior logic_print =
{
	.base.get_gate_name = logx_bool_get_gate_name,
	.base.create = logx_bool_create,
	.base.destroy = lxc_destroy_simple_free,
	.base.get_input_types = logx_bool_get_supported_types,
	.base.get_input_label = logx_bool_get_input_label,
	.base.get_input_max_index = return_20,
	.base.get_input_wire = logx_bool_get_input_wire,
	.base.wire_input = logx_bool_wire_input,
	.base.input_value_changed = logx_bool_input_value_changed,
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

	.base.input_value_changed = dbg_print_exec,
	.name = "logic_print",
	.logic_function = NULL,
};
*/
static void init_behavior
(
	struct lxc_bool_gate_behavior* b,
	const char* gate_name,
	bool(*logic_function)(Gate))
{
	memcpy(b, &commons, sizeof(commons));
	b->name = gate_name;
	b->logic_function = logic_function;
}

static int library_operation_function(enum library_operation op, char*** errors)
{
	if(library_before_load == op)
	{
		//printf("preloading library\n");
		type_bool = lxc_get_signal_by_name("bool");
		if(NULL == type_bool)
		{
			//refuse library load
			array_pnt_append_element((void***) errors, "Can't find `bool` signal type.");
			return 1;
		}

		bool_value_true = lxc_get_constant_by_name("true");
		bool_value_false = lxc_get_constant_by_name("false");

		if(NULL == bool_value_true)
		{
			array_pnt_append_element((void***) errors, "Can't find `true` bool value.");
			return 1;
		}

		if(NULL == bool_value_false)
		{
			array_pnt_append_element((void***) errors, "Can't find `false` bool value.");
			return 1;
		}


		init_behavior(&logic_nand, "nand", logic_function_nand);
		init_behavior(&logic_nor, "nor", logic_function_nor);
		init_behavior(&logic_and, "and", logic_function_and);
		init_behavior(&logic_or, "or", logic_function_or);
		return 0;
	}


	/*else if(library_after_unloaded == op)
	{
		return 0;
	}*/

	return 0;
}

//loadable library definition
struct loadable_library logxcontroll_loadable_library_bool =
{
	.library_operation = library_operation_function,
	.gates = (struct detailed_gate_entry*[])
	{
		&detail_logic_nand,
		&detail_logic_nor,
		&detail_logic_and,
		&detail_logic_or,

		NULL,
	},


};
