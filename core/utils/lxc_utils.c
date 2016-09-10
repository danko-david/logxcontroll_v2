/*
 * lxc_utils.c
 *
 *  Created on: 2016.03.29.
 *      Author: szupervigyor
 */

#include "core/logxcontroll.h"


bool lxc_import_new_value(LxcValue new_val, LxcValue* internal_location)
{
	if(new_val == *internal_location)
	{
		return false;
	}

	if(NULL != new_val)
	{
		lxc_reference_value(new_val);
	}

	if(NULL != *internal_location)
	{
		lxc_unreference_value(*internal_location);
	}

	*internal_location = new_val;

	return true;
}

int lxc_value_size(LxcValue val)
{
	if(NULL == val)
	{
		return 0;
	}

	size_t (*size)(LxcValue) = val->operations->size;
	if(NULL == size)
	{
		return 0;
	}

	return size(val);
}


bool lxc_wipe_value(LxcValue* val)
{
	if(NULL != *val)
	{
		lxc_unreference_value(*val);
		*val = NULL;
		return true;
	}

	return false;
}

LxcValue lxc_create_system_event
(
	enum lxc_system_event_type type,
	Signal signal,
	int subtype,
	int index,
	const char* name
)
{
	LxcValue ret = lxc_create_generic_value
	(
		&lxc_signal_system,
		sizeof(struct lxc_system_event)
	);

	struct lxc_system_event* event =
			(struct lxc_system_event*) lxc_get_value(ret);

	event->event_type = type;
	event->signal = signal;
	event->subtype = subtype;
	event->index = index;
	event->name = name;

	return ret;
}

void lxc_portb_republish_internal_value
(
	Gate gate,
	Signal signal,
	int subtype,
	int index,
	LxcValue* (*access_internal_value)(Gate, int wire_abs)
)
{
	int abs =	lxc_portb_get_absindex
				(
					((struct lxc_generic_portb_instance*)gate),
					DIRECTION_OUT,
					signal,
					subtype,
					index
				);

	if(abs < 0)
	{
		return;
	}

	LxcValue* addr = access_internal_value(gate, abs);
	if(NULL == addr)
	{
		return;
	}

	Tokenport out = ((struct lxc_generic_portb_instance*)gate)->inputs[abs];
	if(NULL == out)
	{
		return;
	}

	lxc_drive_wire_value((Gate) gate, index, out->owner, *addr);
}

LxcValue lxc_get_value_from_tokenport_array
(
	Tokenport* tokenports,
	int index
)
{
	Tokenport w = tokenports[index];
	if(NULL == w)
	{
		return NULL;
	}

	return lxc_get_token_value(w);
}

LxcValue lxc_get_value_safe_from_tokenport_array
(
	Tokenport* tokenports,
	int max_length,
	int index
)
{
	if(index >= max_length)
	{
		return NULL;
	}

	Tokenport w = tokenports[index];
	if(NULL == w)
	{
		return NULL;
	}

	return lxc_get_token_value(w);
}

