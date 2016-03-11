/*
 * builtin_gate_cast.c
 *
 *  Created on: 2016.02.27.
 *      Author: szupervigyor
 */


#include "core/logxcontroll.h"
#include "core/builtins/builtin_gate_cast.h"

const char*** paths =
{
	(char*[]){"built in", "types", NULL},
	NULL
};

struct lxc_cast_instance
{
	struct lxc_instance base;
	Wire input;
	Wire output;
	Signal source;
	Signal destination;
};

static const char* cast_get_gate_name(Gate unused)
{
	UNUSED(unused);
	return "cast to";
}

static Gate cast_create(const struct lxc_gate_behavior* this_behavoir)
{
	struct lxc_instance* ret = malloc_zero(sizeof(struct lxc_cast_instance));
	lxc_init_instance(ret, this_behavoir);
	return ret;
}

static void cast_destroy(Gate instance)
{
	free(instance);
}

static int cast_get_input_types(Gate instance, Signal* arr, uint max_length)
{
	struct lxc_cast_instance* in = (struct lxc_cast_instance*) instance;
	if(NULL != in->source)
	{
		if(max_length < 1)
			return -1;

		arr[0] = in->source;
		return 1;
	}

	return 0;
}

static const char* cast_get_input_label(Gate instance, Signal signal, uint index)
{

}
/*
static const struct lxc_gate_behavior builtin_gate_cast =
{
	.get_gate_name		= cast_get_gate_name,
	.create				= cast_create,
	.destroy			= cast_destroy,
	.get_input_types	= cast_get_input_types,

	//get the input user friandly name
	const char* (*get_input_label)(Gate instance, Signal signal, uint index);

	//get the max index of the specified type, returns negative value if
	//type not supported
	int (*get_input_max_index)(Gate instance, Signal type);

	//returns the wire of the specified type and input. returns null if
	//type not supported or port is not wired
	Wire (*get_input_wire)(Gate instance, Signal type, uint index);

	//\\tries to wire the input. Signal parameter is redundant, if wire is not null
	//\\it will be silently ignored, if wire is null gate unwire the input specified
	//\\by type and index.

	//simply set the wire in the internal data structure specified by signal
	int (*wire_input)(Gate instance, Signal signal, Wire wire, uint index);

	//here notified if an input value changed, you can decide do you want to
	//care about (sensitivity), if you want to execute them without any
	//modification call instance->execution_behavior(instance,type,value,index)
	void (*input_value_changed)(Gate instance, Signal type, LxcValue value, uint index);

	//execute the gate specific function
	//this will be called by the input_value_changed function as it implemented
	//and by the framework if:
	// a new input wired,
	// an input unwired,
	// a new output wired
	// gate enabled,
	//
	//TODO etc.
	//TODO precise doc how does it do.
	void (*execute)(Gate instance, Signal type, LxcValue value, uint index);


	int (*get_output_types)(Gate instance, Signal* arr, uint max_length);

	const char* (*get_output_label)(Gate instance, Signal signal, uint index);

	//get the max index of the specified type, returns negative value if
	//type not supported
	int (*get_output_max_index)(Gate instance, Signal type);

	//returns the wire of the specified type and output. returns null if
	//type not supported or port is not wired
	Wire (*get_output_wire)(Gate instance, Signal type, uint index);



	//\\tries to wire the input. Signal parameter is redundant if wire is not null
	//\\and it will be silently ignored, if wire is null gate unwires the input specified
	//\\by the signal type and index.

	//simply set the wire in the internal data structure specified by signal
	int (*wire_output)(Gate instance, Signal signal, Wire wire, uint index);

	//TODO get_lock_for_execution
	//a structure of function pointers contains the data for locking
	//mechanism

	//prop: set,get enumerate, label, description
	//
	int (*enumerate_properties)(Gate instance, char** arr, uint max_index);

	const char* (*get_property_label)(Gate instance, char* property);

	const char* (*get_property_description)(Gate instance, char* property);

	int (*get_property_value)(Gate instance, char* property, char* dst, uint max_length);

	//returns zero if property successfilly modified, return negative required length if error
	//buffer is too short to write error description. returns positive value if error ocurred and
	//error description successfully written back.
	int (*set_property)(Gate instance, char* property, char* value, char* err, uint max_length);

	//functionality like ioctl
	int (*gatectl)(Gate instance, unsigned long request, ...);
};
*/
/*
const struct detailed_gate_entry* lib_gate_cast =
{
	//.behavior = &builtin_gate_cast,
	//.paths = paths,
};



static int library_load(enum library_operation op, char*** errors)
{
	//on first operation
	//struct lxc_gate_behavior* ret = malloc();

	return 0;
}

*/



