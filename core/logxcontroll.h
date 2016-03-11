/*
 * logxcontroll.h
 *
 *  Created on: 2016.01.22.
 *      Author: szupervigyor
 */

#ifndef LOGXCONTROLL_H_
#define LOGXCONTROLL_H_

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#include "core/lxc.h"
#include "core/lxc_base_impl.h"
#include "core/lxc_execution.h"
#include "core/accounting.h"
//#include "core/builtins/builtin_gate_switch.h"
#include "core/builtins/gate_generic.h"
#include "core/builtins/builtin.h"
#include "module/logic/lxc_bool_gates.h"

#include "core/utils/utils.h"

void logxcontroll_init_environment();

/**
 * TODO steps:
 * 	- implement some basic gate to make a usability test
 *
 *
 *
 * 	- create
 *
 *
 * TODO list:
 * 	- switch
 * 	- lazy impl
 * 	- struct signals
 *	- loadable libraries
 *
 *
 * 	- some library:
 * 		- libao
 * 		- bsdSock
 * 		- gpio
 *
 * */

/*
 	core:
 		- some basic signal types
		- multithreading support
			(new thread for eg.: accept sockets, read from file descriptor,
				earl version has no epoll, io read managed like in java)
			- execution queues

		- reference counting for values and library elements
		- plugin load (so dlopen)

*/

//TODO add init function here, whitch registers the builtin librry functions

#endif /* LOGXCONTROLL_H_ */
