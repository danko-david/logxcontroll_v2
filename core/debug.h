/*
 * debug.h
 *
 *  Created on: 2016.03.22.
 *      Author: szupervigyor
 */

#ifndef DEBUG_H_
#define DEBUG_H_
#include "core/logxcontroll.h"

void lxc_dbg_gate_print_all(Gate);

void lxc_dbg_print_wire(Wire w);

void lxc_dbg_print_gate_name(Gate);

void lxc_dbg_print_properties(Gate);

void lxc_dbg_print_dot_graph(IOCircuit circ);

void lxc_dbg_print_library_tree(bool leafs);

void lxc_dbg_on_oom();

void linux_print_heap_size();

void dbg_print_messages(char** msgs);

void dbg_crash();

void lxc_dbg_print_library_tree(bool leafs);

void dbg_busy_wait_sec(int sec);

#endif /* DEBUG_H_ */
