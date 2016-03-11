/*
 * builtin_gate_cast.h
 *
 *  Created on: 2016.02.27.
 *      Author: szupervigyor
 */

#ifndef BUILTIN_GATE_CAST_H_
#define BUILTIN_GATE_CAST_H_

#include "core/logxcontroll.h"

extern const struct detailed_gate_entry* lib_gate_cast;

int library_load(enum library_operation op, char*** errors);



#endif /* BUILTIN_GATE_CAST_H_ */
