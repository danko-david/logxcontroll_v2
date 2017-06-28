/*
 * debug.c
 *
 *  Created on: 2016.03.22.
 *      Author: szupervigyor
 */

#include "core/logxcontroll.h"

void lxc_dbg_print_gate_name(Gate gate)
{
	printf("Gate name:%s\n", lxc_gate_get_name(gate));
}

void lxc_dbg_print_properties(Gate gate)
{
	if(NULL == gate)
	{
		printf("Gate (null) property is empty\n");
		return;
	}

	printf("Gate (%s) properties:\n", lxc_gate_get_name(gate));
	const char* props[20];
	int len = lxc_gate_enumerate_properties(gate, props, 20);
	char buf[200];
	int i;
	for(i=0;i<len;++i)
	{
		lxc_gate_get_property_value(gate, props[i], buf, 200);
		printf("\t%s: \"%s\"\n", props[i], buf);
	}
	printf("\n");
}

void lxc_dbg_gate_print_all(Gate g)
{
	if(NULL == g)
	{
		printf("Gate (NULL)\n");
		return;
	}

	printf
	(
		"Gate (%p) type_name: %s, ref_des: %s, enabled: %s\n",
		g,
		g->behavior->gate_name,
		g->ref_des,
		lxc_gate_is_enabled(g)?"true":"false"
	);

	Signal sig[20];
	int sub[20];

	{
		int len = lxc_gate_get_input_types(g, sig, sub, 20);
		if(0 == len)
		{
			printf("no inputs\n");
		}
		else
		{
			printf("input:\n");
			int i=-1;
			while(++i < len)
			{
				int max = lxc_gate_get_input_max_index(g, sig[i], sub[i]);
				int n = -1;
				while(++n < max)
				{
					printf
					(
						"\t%s [%s|%d]: ",
						lxc_gate_get_input_label(g, sig[i], sub[i], n),
						sig[i]->name,
						sub[i]
					);

					Tokenport p = lxc_gate_get_input_port(g, sig[i], sub[i], n);
					lxc_dbg_print_wire(NULL == p? NULL :p->owner);
					printf("\n");
				}
			}
		}
	}

	{
		int len = lxc_gate_get_output_types(g, sig, sub, 20);
		if(0 == len)
		{
			printf("no outputs\n");
		}
		else
		{
			printf("output:\n");
			int i=-1;
			while(++i < len)
			{
				int max = lxc_gate_get_output_max_index(g, sig[i], sub[i]);
				int n = -1;
				while(++n < max)
				{
					printf
					(
						"\t%s [%s|%d]: ",
						lxc_gate_get_output_label(g, sig[i], sub[i], n),
						sig[i]->name,
						sub[i]
					);

					Wire w = lxc_gate_get_output_wire(g, sig[i], sub[i], n);
					lxc_dbg_print_wire(w);
					printf("\n");
				}
			}
		}
	}

	{
		const char* props[20];
		int len = lxc_gate_enumerate_properties(g, props, 20);
		if(0 == len)
		{
			printf("no properties\n");
		}
		else
		{
			char buf[200];
			int i;
			for(i=0;i<len;++i)
			{
				lxc_gate_get_property_value(g, props[i], buf, 200);
				printf("\t%s: \"%s\"\n", props[i], buf);
			}
			printf("\n");
		}
	}

	printf("\n");
}

void lxc_dbg_print_wire(Wire w)
{
	if(NULL == w)
	{
		printf("Wire (NULL)");
	}
	else
	{
		printf
		(
			"Wire(%p): signal: %s, sub: %d, ref_des: %s",
			w,
			w->type->name,
			w->subtype,
			w->ref_des
		);
	}
}

void lxc_dbg_print_value(LxcValue val)
{
	if(NULL == val)
	{
		printf("LxcValue (NULL)");
	}
	else
	{
		int refc = -1024;
		if(NULL != val->operations->ref_diff)
		{
			refc = val->operations->ref_diff(val, 0);
		}

		printf
		(
			"LxcValue(%p): sig: %s, refc: %d",
			val,
			val->type->name,
			refc
		);
	}
}

static void print_wired_ref(Tokenport tp, bool direction, char* def)
{
	const char* (*func)(Gate gate, Signal type, int subtype, uint index) =
	 DIRECTION_IN == direction?
		lxc_gate_get_input_label
	:
		lxc_gate_get_output_label;

	printf
	(
		"\"%s\":\"%s\"",
		NULL == tp?def:tp->gate->ref_des,
		NULL == tp?def:func(tp->gate, tp->owner->type, tp->owner->subtype, tp->index)
	);
}

static int print_dot_wire(Wire w)
{
	Tokenport driver = NULL;

	if(NULL != w->drivers)
	{
		driver = w->drivers[0];
	}

	int d = 0;
	if(0 == w->drivens_length)
	{
		printf("\t");
		print_wired_ref(driver, false, w->ref_des);
		printf(" -> ");
		print_wired_ref(NULL, true, w->ref_des);
		printf("\n");
	}
	else
	{
		while(d < w->drivens_length)
		{
			printf("\t");
			print_wired_ref(driver, false, w->ref_des);
			printf(" -> ");
			print_wired_ref(w->drivens[d], true, w->ref_des);
			printf("\n");
			++d;
		}
	}

	return 0;
}

static int print_dot_gate(Gate g)
{
	printf
	(
		"\t\"%s\" [shape=record,label=\"",
		g->ref_des
	);

	char** i_names = NULL;
	char** o_names = NULL;

	lxc_gate_enumerate_input_labels_into(&i_names, g);
	lxc_gate_enumerate_output_labels_into(&o_names, g);

	int i_size = array_pnt_population((void**)i_names);
	int o_size = array_pnt_population((void**)o_names);

	int max;

	if(i_size > o_size)
	{
		max = i_size;
	}
	else
	{
		max = o_size;
	}

	if(max < 1)
	{
		max = 1;
	}

	//"G 4" [shape=record,label="{<I0>I0|sort_box|<O0>O0}|{<I1>I1|<O1>O1}|{<I2>I2|<O2>O2}|{|O3}"];

	int i = -1;
	while(++i < max)
	{
		if(0 != i)
		{
			printf("|");
		}

		printf("{");

		if(i < i_size)
		{
			char* n = i_names[i];
			printf("<%s> %s", n, n);
		}

		if(0 == i)
		{
			printf("| %s [%s]", g->behavior->gate_name, g->ref_des);
		}

		if(i < o_size)
		{
			char* n = o_names[i];
			printf("|<%s> %s", n, n);
		}
		else
		{
			printf("|");
		}

		printf("}");
	}

	printf("\"];\n");

	free(i_names);
	free(o_names);

}


void lxc_dbg_print_dot_graph(IOCircuit circ)
{
	printf("digraph structs {\n");
	printf("	rankdir=LR\n");
	printf("	node [shape=record];\n");

	hashmap_iterate(circ->gates, print_dot_gate, NULL);

	printf("\n");

	hashmap_iterate(circ->wires, print_dot_wire, NULL);

	printf("}\n");

}

#include <fcntl.h>

void linux_print_heap_size()
{
	char data[2048];
	int fd = open("/proc/self/maps", O_RDONLY);

	read(fd, data, sizeof(data));

	data[sizeof(data)-1] = '0';

	close(fd);

	char* off = strstr(data, "[heap]");

	if(NULL == off)
	{
		printf("no heap initiated\n");
		return;
	}

	while(*off != '\n')
		--off;

	char* sign = strstr(off, "-");

	long low = strtoul(off+1, NULL, 16);

	long high = strtoul(sign+1, NULL, 16);

	long diff = high - low;

	printf("Heap size: %ld Mb, %ld Kb, %ld bytes\n", diff/(1024*1024), diff/1024, diff);
}

#include <execinfo.h>

/**
 *
 * https://www.gnu.org/software/libc/manual/html_node/Backtraces.html
 * */
void gnu_libc_print_stack_trace()
{
	void *array[10];
	size_t size;
	char **strings;
	size_t i;

	size = backtrace(array, 10);
	strings = backtrace_symbols(array, size);

	printf ("Obtained %zd stack frames.\n", size);

	for (i = 0; i < size; i++)
	{
		printf ("%s\n", strings[i]);
	}

	free (strings);
}

void gnu_libc_print_stack_trace_then_terminalte()
{
	gnu_libc_print_stack_trace();
	exit(11);
}

void dbg_print_messages(char** msgs)
{
	if(NULL == msgs)
		return;

	int i;
	for(i=0;NULL != msgs[i];++i)
	{
		printf("%s\n", msgs[i]);
	}
}

void dbg_crash()
{
	int* val = NULL;
	*val = 0;
}

void lxc_dbg_on_oom()
{
	printf("Out of memory ocurred.");
	gnu_libc_print_stack_trace_then_terminalte();
}
