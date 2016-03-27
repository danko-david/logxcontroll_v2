/*
 * lxc_bool_gates.h
 *
 *  Created on: 2016.01.22.
 *      Author: szupervigyor
 */

#ifndef LXC_BOOL_GATES_H_
#define LXC_BOOL_GATES_H_

#include "core/logxcontroll.h"

struct lxc_bool_gate_behavior
{
	const struct lxc_gate_behavior base;
	bool (*logic_function)(Gate);
};

struct lxc_bool_instance
{
	struct lxc_instance base;
	Wire inputs[21];
	Wire output;
};

extern struct lxc_loadable_library logxcontroll_loadable_library_bool;

#endif /* LXC_BOOL_GATES_H_ */
