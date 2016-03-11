/*
 * builtin_gate_switch.h
 *
 *  Created on: 2016.02.18.
 *      Author: szupervigyor
 */

#include "core/logxcontroll.h"

#ifndef BUILTIN_GATE_SWITCH_H_
#define BUILTIN_GATE_SWITCH_H_

extern const struct lxc_gate_behavior builtin_gate_switch;

//enumerate inputs
//enumerate outputs

enum switch_ctl_request
{
	SWITCH_CTL_ADD_INPUT,//name, type
	SWITCH_CTL_ADD_OUTPUT,//name, type

	SWITCH_CTL_MODIFY_PORT,//name, kind, union{name, value, direction}
	SWITCH_CTL_REMOVE_PORT,//name

	SWITCH_CTL_CREATE_CASE,//name, value,
	SWITCH_CTL_SET_OF_CASE,//name, kind, union{name, value}
	SWITCH_CTL_GET_OF_CASE,//name, kind, union{name, value, iocircuit}
	SWITCH_CTL_REMOVE_CASE,//name
};

enum switch_ctl_kind
{
	SWITCH_CTL_KIND_NAME,
	SWITCH_CTL_KIND_TYPE,
	SWITCH_CTL_KIND_VALUE,
	SWITCH_CTL_KIND_DIRECTION,
	SWITCH_CTL_KIND_CIRCUIT,
};
//enumerate cases

/**
 * switch special operations:
 *	- add/remove/specify
 *		input or output
 *			name or type
 *
 *	- get/set switch type
 *	- add/remove case
 *	- set/get case value
 *	- get case iocircuit address
 *
 *
 * */
struct switchctl
{
	char* name;
	Signal type;
	enum switch_ctl_kind kind;
	union
	{
		char* new_name;
		bool direction;
		LxcValue value;
		IOCircuit circuit;
	};
};


#endif /* BUILTIN_GATE_SWITCH_H_ */
