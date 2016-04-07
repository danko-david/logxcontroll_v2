/*
 * lxc_utils.h
 *
 *  Created on: 2016.03.29.
 *      Author: szupervigyor
 */

#ifndef LXC_UTILS_H_
#define LXC_UTILS_H_
#include "core/logxcontroll.h"

/**
 * tires to `import` new value behind the gate logic,
 * if old value is not null, it will be unreferenced,
 * new value will be referenced (if not null) and placed
 * to the specified location.
 *
 * if old val and internal value is the same (identically) nothing happens
 * and false returns, otherwise operation executed and true returned
 * */
bool lxc_import_new_value(LxcValue, LxcValue*);

int lxc_value_size(LxcValue);

/**
 * tries to wipe out value from the specified location.
 * unreference the value
 * and update to NULL
 **/
bool lxc_wipe_value(LxcValue*);

LxcValue lxc_create_system_event
(
	enum lxc_system_event_type type,
	Signal signal,
	int index,
	const char* name
);

void lxc_portb_republish_internal_value
(
	Gate gate,
	Signal signal,
	int index,
	LxcValue* (access_internal_value)(Gate, int wire_abs)
);

LxcValue lxc_get_value_safe_from_wire_array
(
	Wire* wires,
	int max_length,
	int index
);

#endif /* LXC_UTILS_H_ */
