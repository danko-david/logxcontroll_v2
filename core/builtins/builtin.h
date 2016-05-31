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


extern const struct lxc_signal_type lxc_signal_system;
extern const struct lxc_signal_type lxc_signal_pulse;
extern const struct lxc_signal_type lxc_signal_bool;
extern const struct lxc_signal_type lxc_signal_byte;
extern const struct lxc_signal_type lxc_signal_short;
extern const struct lxc_signal_type lxc_signal_int;
extern const struct lxc_signal_type lxc_signal_float;
extern const struct lxc_signal_type lxc_signal_long;
extern const struct lxc_signal_type lxc_signal_double;

extern const struct lxc_signal_type lxc_signal_string;

extern const struct lxc_signal_type lxc_signal_data;

extern const struct lxc_constant_value lxc_bool_constant_value_true;

extern const struct lxc_constant_value lxc_bool_constant_value_false;

struct lxc_primitive_value
{
	struct lxc_value base;
	int refcount;
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

#include "core/lxc.h"
//#include "core/lxc_base_impl.h"


extern const struct lxc_loadable_library logxcontroll_loadable_library_builtin;

/**************************** Built in gates **********************************/

extern const char*** lxc_built_in_path_type;
extern const char*** lxc_built_in_path_value_propagation;

//cast

extern struct lxc_gate_behavior lxc_built_in_gate_cast;
void lxc_builtin_cast_init_before_load();

#endif /* BUILTIN_H_ */
