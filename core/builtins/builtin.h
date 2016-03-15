/*
 * builtin.h
 *
 *  Created on: 2016.02.27.
 *      Author: szupervigyor
 */

#ifndef BUILTIN_H_
#define BUILTIN_H_

#include "core/logxcontroll.h"

/**
 * builtin types:
 * 	- bool
 *	- byte
 * 	- int
 * 	- float
 * 	- long
 * 	- double
 *	========
 *	- string
 *	- data (char[])
 *
 *	========
 *	- pcm
 *	- image
 *
 *	========
 *	- data_array
 *	- data_object
 *
 *
 * built_in library (gates):
 * 	- cast_to
 * 	- struct
 * 	- struct_access
 * 	- to_array
 * 	- array_access
 * 	- case
 * 	- loop
 * 	- string driven port
 *	- string mux/demux
 *	- readline
 *	- sprintf
 *	- send/receive value
 *	- posix sockets!
 *		- detail
 *
 *
 * self usage
 *  - select workspace
 *  	- get circuit => iocircuit
 *
 *  - iocircuit:
 *  	- create wire
 *  	- create gate
 *  	- wiring operation (get_value, set_value, destroy, wire_gate_input, wire_gate_output)
 *
 *
 * */

struct lxc_primitive_value
{
	struct lxc_value base;
	union
	{
		char	char_value;

		short	short_value;

		int		int_value;
		float	float_value;

		long	long_value;
		double	double_value;
	};
};


extern const struct lxc_value_operation primitive_constant_value_operations;
extern const struct lxc_value_operation primitive_variable_value_operations;

LxcValue lxc_create_primitive_value(Signal type);

void lxc_free_primitive_value(LxcValue value);

LxcValue lxc_clone_primitive_value(LxcValue value);

size_t lxc_size_primitive_value(/*LxcValue value*/);

int lxc_reference_primitive_value(LxcValue val);

int lxc_unreference_primitive_value(LxcValue val);

void* lxc_data_address_primitive_value(LxcValue val);

#include "core/lxc.h"
#include "core/lxc_base_impl.h"


void lxc_init_instance(Gate instance, const struct lxc_gate_behavior* behavior);


extern const struct loadable_library logxcontroll_loadable_library_builtin;

/**************************** Built in gates **********************************/

extern const char*** lxc_built_in_path_type;
extern const char*** lxc_built_in_path_value_propagation;

//cast

extern struct detailed_gate_entry lxc_built_in_gate_cast;
void lxc_builtin_cast_init_before_load();

#endif /* BUILTIN_H_ */
