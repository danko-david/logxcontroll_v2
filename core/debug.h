/*
 * debug.h
 *
 *  Created on: 2016.03.22.
 *      Author: szupervigyor
 */

#ifndef DEBUG_H_
#define DEBUG_H_
#include "core/logxcontroll.h"


void lxc_dbg_print_gate_name(Gate);

void lxc_dbg_print_properties(Gate);

void lxc_dbg_print_library_tree(bool leafs);

#endif /* DEBUG_H_ */
