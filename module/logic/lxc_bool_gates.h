/*
 * lxc_bool_gates.h
 *
 *  Created on: 2016.01.22.
 *      Author: szupervigyor
 */

#ifndef LXC_BOOL_GATES_H_
#define LXC_BOOL_GATES_H_

#include "core/logxcontroll.h"

#include <stdbool.h>

struct lxc_bool_gate_behavior
{
	const struct lxc_gate_behavior base;
	const char* name;
	bool (*logic_function)(Gate);
};

struct lxc_bool_instance
{
	struct lxc_instance base;
	Wire inputs[21];
	Wire output;
};

Signal type_bool;
LxcValue bool_value_false;
LxcValue bool_value_true;

extern struct loadable_library logxcontroll_loadable_library_bool;

#endif /* LXC_BOOL_GATES_H_ */
