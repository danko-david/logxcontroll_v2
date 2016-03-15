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

#include <unistd.h>

#include <string.h>
#include <errno.h>


#include "core/lxc.h"
#include "core/lxc_base_impl.h"
#include "core/lxc_execution.h"
#include "core/accounting.h"
#include "core/builtins/gate_generic.h"
#include "core/builtins/builtin.h"
#include "core/builtins/logic/lxc_bool_gates.h"

#include "core/utils/utils.h"

void logxcontroll_init_environment();

/**
 * TODO steps:
 *	- create generic gate implementation utility:
 *		- port manager
 *		- property manager
 *
 *	- create basic library environment like:
 *		- workspace
 *		- search and creator surface:
 *			- search for:
 *				- gate, wire by path (workspace/named_gate, workspace/gate/case_12/named_gate)
 *
 *
 * 	- implement some basic gate to make a usability test
 *
 *	- implement functions for RPC propuses
 *
 *	- make a java binding and
 *
 *
 * 	- create
 *
 * libraries to implement:
 * 	- built in gates:
 * 		- cast to (cast value type to another specified type see: Signal->cast_to**)
 * 		- const (propagate a value to the output from the constant pool)
 *
 * 	- POSIX based gates:
 * 		- fd io (file descriptor read and write)
 * 		- socket bring up (connect/listen operations for local & remote sockaddr)
 * 		- socket create (and constants like AF_INET, AF_UNIX)
 *			- TODO discover more function like: AF_BLUETOOTH, AF_PACKET
 *		- serial (/dev/tty* )
 *
 * 	- linux specific gates:
 * 		- proc gpio (linux's /proc/sys/class/gpio ineterface)
 * 		- capture audio
 * 		- v4l2 (/dev/video* capture)
 *
 *	- other library bindings:
 *		- libao
 *
 *
 *
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
