/*
 * debug.c
 *
 *  Created on: 2016.03.22.
 *      Author: szupervigyor
 */

#include "core/logxcontroll.h"

void lxc_dbg_print_gate_name(Gate gate)
{
	printf("Gate name:%s\n", lxc_get_gate_name(gate));
}

void lxc_dbg_print_properties(Gate gate)
{
	if(NULL == gate)
	{
		printf("Gate (null) property is empty\n");
		return;
	}

	printf("Gate (%s) properties:\n", lxc_get_gate_name(gate));
	const char* props[20];
	int len = lxc_enumerate_properties(gate, props, 20);
	char buf[200];
	for(int i=0;i<len;++i)
	{
		lxc_get_property_value(gate, props[i], buf, 200);
		printf("\t%s: \"%s\"\n", props[i], buf);
	}
	printf("\n");
}

void lxc_dbg_print_library_tree(bool leafs);
