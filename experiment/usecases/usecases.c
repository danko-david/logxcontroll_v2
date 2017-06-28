
#include "core/logxcontroll.h"


/****************************** usecase register ******************************/

struct case_option
{
	const char* name;
	void (*funct)();
};

static struct case_option** OPTS = NULL;

void register_option(const char* name, void (*funct))
{
	struct case_option* add = malloc(sizeof(struct case_option));
	add->name = name;
	add->funct = funct;
	array_pnt_append_element
	(
		(void***) &OPTS,
		(void*)add
	);
}



/**************************** Generic functions *******************************/

static Wire get_or_create_wire
(
	IOCircuit circ,
	char* name,
	Signal sig
)
{
	Wire in = lxc_circuit_get_wire_by_refdes(circ, name);
	if(NULL == in)
	{
		in = lxc_wire_create(sig);
		lxc_wire_set_refdes(in, name);
		lxc_circuit_add_wire(circ, in);
	}

	return in;
}

struct lxc_generic_portb_behavior* create_portb_behavior
(
	char* gate_name,
	int size,
	void (*execute)(Gate instance, Signal type, int subtype, LxcValue value, uint index)
)
{
	struct lxc_generic_portb_behavior* ret = malloc_zero(sizeof(struct lxc_generic_portb_behavior));
	lxc_init_from_prototype
	(
		ret,
		sizeof(struct lxc_generic_portb_behavior),

		(void*)&lxc_generic_portb_prototype,
		sizeof(lxc_generic_portb_prototype)
	);

	char** name = (char**) &ret->base.gate_name;
	*name = gate_name;

	Signal t_int = &lxc_signal_int;

	ret->instance_memory_size = size;
	ret->base.execute = execute;

	return ret;
}

struct lxc_generic_porti_behavior* create_porti_behavior
(
	char* gate_name,
	int size,
	void (*execute)(Gate instance, Signal type, int subtype, LxcValue value, uint index)
)
{
	struct lxc_generic_porti_behavior* ret = malloc_zero(sizeof(struct lxc_generic_porti_behavior));
	lxc_init_from_prototype
	(
		ret,
		sizeof(struct lxc_generic_porti_behavior),

		(void*)&lxc_generic_porti_prototype,
		sizeof(lxc_generic_porti_prototype)
	);

	char** name = (char**) &ret->base.gate_name;
	*name = gate_name;

	ret->instance_memory_size = size;
	ret->base.execute = execute;

	return ret;
}

/*********************** Computerphile's sorter box impl **********************/

//ordinal function
static int triangle_nth(int i, int j)
{
	return (i*(i+1))/2 +j;
}

//=====> porti
struct lxc_generic_porti_behavior* porti_sorter_box;

static void porti_sorter_exec
(
	Gate instance, Signal type, int subtype, LxcValue value, uint index
)
{
	struct lxc_generic_porti_instance* g =
		(struct lxc_generic_porti_instance*) instance;

	Tokenport up = array_fix_try_get((void**) g->inputs, g->inputs_length, 0);
	Tokenport left = array_fix_try_get((void**) g->inputs, g->inputs_length, 1);

	//none available
	if(!lxc_wire_token_available(up) && !lxc_wire_token_available(left))
	{
		return;
	}

	LxcValue a = lxc_get_token_value(up);
	LxcValue b = lxc_get_token_value(left);

	//input wired, but value not present
	if
	(
		(NULL != up && NULL == a)
		||
		(NULL != left && NULL == b)
	)
	{
		goto go_return;
		return;
	}

	if(NULL != a)
	{
		lxc_absorb_token(up);
	}

	if(NULL != b)
	{
		lxc_absorb_token(left);
	}

	LxcValue go_down = NULL;
	LxcValue go_right = NULL;

	if(NULL != a && NULL != b)
	{
		int* v_a = lxc_get_value(a);
		int* v_b = lxc_get_value(b);

		if(*v_a < *v_b)
		{
			go_down = a;
			go_right = b;
		}
		else
		{
			go_down = b;
			go_right = a;
		}
	}
	else
	{
		if(NULL != a)
			go_down = a;
		else
			go_down = b;
	}

	Wire down = array_fix_try_get((void**) g->outputs, g->outputs_length, 0);
	Wire right = array_fix_try_get((void**) g->outputs, g->outputs_length, 1);

	lxc_drive_wire_value(instance, 0, down, go_down);
	lxc_drive_wire_value(instance, 1, right, go_right);

go_return:
	lxc_wire_release_token(up);
	lxc_wire_release_token(left);

	//lxc_import_new_value(NULL, &a);
	//lxc_import_new_value(NULL, &b);
}


static struct lxc_generic_porti_behavior* create_comphile_sorter_behavior
(
	char* gate_name
)
{
	struct lxc_generic_porti_behavior* ret = create_porti_behavior
	(
		"sorter_box",
		sizeof(struct lxc_generic_porti_instance),
		porti_sorter_exec
	);


	return ret;
}


static Gate porti_comparator_create()
{
	if(NULL == porti_sorter_box)
	{
		porti_sorter_box = create_porti_behavior
		(
			"sorter_box",
			sizeof(struct lxc_generic_porti_instance),
			porti_sorter_exec
		);
	}

	Signal t_int = &lxc_signal_int;

	struct lxc_generic_porti_instance* g =
		(struct lxc_generic_porti_instance*)
		porti_sorter_box->base.create(&porti_sorter_box->base);

	lxc_port_unchecked_add_new_port(&g->input_ports, "I0", t_int, 0, NULL);
	lxc_port_unchecked_add_new_port(&g->input_ports, "I1", t_int, 0, NULL);

	lxc_port_unchecked_add_new_port(&g->output_ports, "Q0", t_int, 0, NULL);
	lxc_port_unchecked_add_new_port(&g->output_ports, "Q1", t_int, 0, NULL);

	return &g->base;
}


//=====> portb


static void portb_sorter_exec
(
	Gate instance, Signal type, int subtype, LxcValue value, uint index
)
{
	struct lxc_generic_portb_instance* g =
		(struct lxc_generic_portb_instance*) instance;

	Tokenport up = array_fix_try_get((void**) g->inputs, 2, 0);
	Tokenport left = array_fix_try_get((void**) g->inputs, 2, 1);

	//none available
	if(!lxc_wire_token_available(up) && !lxc_wire_token_available(left))
	{
		return;
	}

	LxcValue a = lxc_get_token_value(up);
	LxcValue b = lxc_get_token_value(left);

	//input wired, but value not present
	if
	(
		(NULL != up && NULL == a)
		||
		(NULL != left && NULL == b)
	)
	{
		goto go_return;
		return;
	}

	if(NULL != a)
	{
		lxc_absorb_token(up);
	}

	if(NULL != b)
	{
		lxc_absorb_token(left);
	}

	LxcValue go_down = NULL;
	LxcValue go_right = NULL;

	if(NULL != a && NULL != b)
	{
		int* v_a = lxc_get_value(a);
		int* v_b = lxc_get_value(b);

		if(*v_a < *v_b)
		{
			go_down = a;
			go_right = b;
		}
		else
		{
			go_down = b;
			go_right = a;
		}
	}
	else
	{
		if(NULL != a)
			go_down = a;
		else
			go_down = b;
	}

	Wire down = array_fix_try_get((void**) g->outputs, 2, 0);
	Wire right = array_fix_try_get((void**) g->outputs, 2, 1);

	lxc_drive_wire_value(instance, 0, down, go_down);
	lxc_drive_wire_value(instance, 1, right, go_right);

go_return:
	lxc_wire_release_token(up);
	lxc_wire_release_token(left);

	//lxc_import_new_value(NULL, &a);
	//lxc_import_new_value(NULL, &b);
}

struct lxc_generic_portb_behavior* portb_sorter_box;


static Gate portb_comparator_create()
{
	if(NULL == portb_sorter_box)
	{
		portb_sorter_box = create_portb_behavior
		(
			"sorter_b box",
			sizeof(struct lxc_generic_portb_instance),
			portb_sorter_exec
		);

		Signal t_int = &lxc_signal_int;

		struct lxc_generic_portb_behavior* g = portb_sorter_box;
		lxc_port_unchecked_add_new_port(&g->input_ports, "I0", t_int, 0, NULL);
		lxc_port_unchecked_add_new_port(&g->input_ports, "I1", t_int, 0, NULL);

		lxc_port_unchecked_add_new_port(&g->output_ports, "Q0", t_int, 0, NULL);
		lxc_port_unchecked_add_new_port(&g->output_ports, "Q1", t_int, 0, NULL);

	}

	return portb_sorter_box->base.create(&portb_sorter_box->base);
}


//=====> spec

struct computerphile_sotert_box_instance
{
	struct lxc_instance base;
	Tokenport in[2];
	Wire out[2];
};

static void spec_sorter_exec
(
	Gate instance, Signal type, int subtype, LxcValue value, uint index
)
{
	struct computerphile_sotert_box_instance* g =
		(struct computerphile_sotert_box_instance*) instance;

	Tokenport up = array_fix_try_get((void**) g->in, 2, 0);
	Tokenport left = array_fix_try_get((void**) g->in, 2, 1);

	//none available
	if(!lxc_wire_token_available(up) && !lxc_wire_token_available(left))
	{
		return;
	}

	LxcValue a = lxc_get_token_value(up);
	LxcValue b = lxc_get_token_value(left);

	//input wired, but value not present
	if
	(
		(NULL != up && NULL == a)
		||
		(NULL != left && NULL == b)
	)
	{
		goto go_return;
		return;
	}

	if(NULL != a)
	{
		lxc_absorb_token(up);
	}

	if(NULL != b)
	{
		lxc_absorb_token(left);
	}

	LxcValue go_down = NULL;
	LxcValue go_right = NULL;

	if(NULL != a && NULL != b)
	{
		int* v_a = lxc_get_value(a);
		int* v_b = lxc_get_value(b);

		if(*v_a < *v_b)
		{
			go_down = a;
			go_right = b;
		}
		else
		{
			go_down = b;
			go_right = a;
		}
	}
	else
	{
		if(NULL != a)
			go_down = a;
		else
			go_down = b;
	}

	Wire down = array_fix_try_get((void**) g->out, 2, 0);
	Wire right = array_fix_try_get((void**) g->out, 2, 1);

	lxc_drive_wire_value(instance, 0, down, go_down);
	lxc_drive_wire_value(instance, 1, right, go_right);

go_return:
	lxc_wire_release_token(up);
	lxc_wire_release_token(left);

	//lxc_import_new_value(NULL, &a);
	//lxc_import_new_value(NULL, &b);
}

struct lxc_generic_portb_behavior* spec_sorter_box;

static Gate spec_create(const struct lxc_gate_behavior* beh)
{
	Gate ret = malloc_zero(sizeof(struct computerphile_sotert_box_instance));
	lxc_init_instance(ret, beh);
	return ret;
}

static Tokenport spec_get_input_wire(Gate instance, Signal type, int subtype, uint index)
{
	return ((struct computerphile_sotert_box_instance*) instance)->in[index];
}

static int spec_wire_input(Gate instance, Signal sig, int subtype, Tokenport port, uint index)
{
	((struct computerphile_sotert_box_instance*) instance)->in[index] = port;
	return 0;
}

static Wire spec_get_output_wire(Gate instance, Signal type, int subtype, uint index)
{
	return ((struct computerphile_sotert_box_instance*) instance)->out[index];
}

static int spec_wire_output(Gate instance, Signal type, int subtype, Wire wire, uint port)
{
	((struct computerphile_sotert_box_instance*) instance)->out[port] = wire;
	return 0;
}

static Gate spec_comparator_create()
{
	//the hacky way
	if(NULL == spec_sorter_box)
	{
		spec_sorter_box = create_portb_behavior
		(
			"sorter_s box",
			sizeof(struct computerphile_sotert_box_instance),
			spec_sorter_exec
		);

		Signal t_int = &lxc_signal_int;

		struct lxc_generic_portb_behavior* g = spec_sorter_box;
		lxc_port_unchecked_add_new_port(&g->input_ports, "I0", t_int, 0, NULL);
		lxc_port_unchecked_add_new_port(&g->input_ports, "I1", t_int, 0, NULL);

		lxc_port_unchecked_add_new_port(&g->output_ports, "Q0", t_int, 0, NULL);
		lxc_port_unchecked_add_new_port(&g->output_ports, "Q1", t_int, 0, NULL);

		//wiring create/destroy, in/out, exec
		g->base.create = spec_create;
		g->base.destroy = lxc_gate_destroy_simple_free;

		g->base.wire_input = spec_wire_input;
		g->base.get_input_wire = spec_get_input_wire;

		g->base.wire_output = spec_wire_output;
		g->base.get_output_wire = spec_get_output_wire;
	}

	return spec_sorter_box->base.create(&spec_sorter_box->base);
}



/******************************************************************************/

/**
 * https://www.youtube.com/watch?v=pcJHkWwjNl4
 *
 * 	  0 1 2 3
 * 0: 0
 * 1: 1	2
 * 2: 3 4 5
 * 3: 6 7 8 9
 * 4: a b c d e
 *
 * 0 = 0x 0y
 * 2 = 1x 1y
 * 5 = 2x 2y
 * 9 = 3x 3y
 *
 * n = (i*(i+1))/2 +j
 *
 *
 * n = n + y;
 *
 *
 *	if( j == 0 ): input wire
 *	if( i == last-1): output wire
 *
 *	node io:
 *		input:
 *			upper: if(i == j): no input, otherwise nth(i-1, j-1);
 *			left: if(0 == j):  i'th network input, otherwise nth(i-1, j-1);
 *		output:
 *			down: if(i == elem-1) j'th output, otherwise nth(i, j)
 *			right: if(i == j) no output, otherwise nth(i+1, j)
 *
 * i0 --A
 * 		|
 * 		y1
 * 		|
 * i1 --B -x1-	C
 * 		|		|
 * 		y2		y3
 * 		|		|
 * i2-- D -x2-	E -x3-	F
 * 		|		|		|
 * 		y4		y5		y6
 * 		|		|		|
 * i3 --G -x4-	H -x5-	I -x6-	J
 * 		|		|		|		|
 * 		o0		o1		o2		o3
 *
 *	number of gates: nth(i, j)+1;
 *	number of wires: (nth(i, j)+1) *2;
 *
 */
IOCircuit create_computerphile_sort_network(Gate (*creator)(), int elem)
{


	int i = -1;
	int j;

	Signal t_int = &lxc_signal_int;

	IOCircuit circ = lxc_create_iocircuit();

	char name[20];

	while(++i < elem)
	{
		j = -1;
		while(++j < i+1)
		{
			Gate g = creator();

			sprintf(name, "G_%d", triangle_nth(i, j));
			lxc_gate_set_refdes(g, name);

			if(0 != lxc_circuit_add_gate(circ, g))
			{
				printf("collision\n");
			}

			/*
			 * 	input:
			 *		upper: if(i == j): no input, otherwise nth(i-1, j-1);
			 *		left: if(0 == j):  i'th network input, otherwise nth(i-1, j-1);
			 *	output:
			 *		down: if(i == elem-1) j'th output, otherwise nth(i, j)
			 *		right: if(i == j) no output, otherwise nth(i+1, j)
			*/

			//upper
			if(i != j)
			{
				sprintf(name, "y%d", triangle_nth(i-1, j-1)+1);
				Wire upper = get_or_create_wire(circ, name, t_int);
				lxc_wire_gate_input(t_int, 0, upper, (Gate) g, 0);
			}

			//left
			{
				if(0 == j)
					sprintf(name, "i%d", i);
				else
					sprintf(name, "x%d", triangle_nth(i-1, j-1));

				Wire left = get_or_create_wire(circ, name, t_int);
				lxc_wire_gate_input(t_int, 0, left, (Gate) g, 1);
			}

			//down
			{
				if(i == elem-1)
					sprintf(name, "o%d", j);
				else
					sprintf(name, "y%d", triangle_nth(i, j));

				Wire down = get_or_create_wire(circ, name, t_int);
				lxc_wire_gate_output(t_int, 0, down, (Gate) g, 0);
			}

			//right
			if(i != j)
			{
				sprintf(name, "x%d", triangle_nth(i-1, j));
				Wire upper = get_or_create_wire(circ, name, t_int);
				lxc_wire_gate_output(t_int, 0, upper, (Gate) g, 1);
			}
		}
	}

	/*
	{
		int i = -1;
		while(NULL != circ->gates[++i])
		{
			lxc_dbg_gate_print_all(circ->gates[i]);
		}
	}
	*/

	/*
	{
		int i = -1;
		while(NULL != circ->wires[++i])
		{
			lxc_dbg_print_wire(circ->wires[i]);
			printf("\n");
		}
	}
	*/

	return circ;
}

/*

porti: 100
Number of gates: 5050
Number of wires: 10100
Heap size: 6 Mb, 6164 Kb, 6311936 bytes

portb: 100
Number of gates: 5050
Number of wires: 10100
Heap size: 4 Mb, 4432 Kb, 4538368 bytes

spec:
Number of gates: 5050
Number of wires: 10100
Heap size: 3 Mb, 3652 Kb, 3739648 bytes


===

porti: 150
Number of gates: 11325
Number of wires: 22650
Heap size: 13 Mb, 13392 Kb, 13713408 bytes

portb: 150
Number of gates: 11325
Number of wires: 22650
Heap size: 9 Mb, 9664 Kb, 9895936 bytes

spec:
Number of gates: 11325
Number of wires: 22650
Heap size: 7 Mb, 7988 Kb, 8179712 bytes

===

porti: 200
Number of gates: 20100
Number of wires: 40200
Heap size: 23 Mb, 23556 Kb, 24121344 bytes


portb: 200
Number of gates: 20100
Number of wires: 40200
Heap size: 14 Mb, 14812 Kb, 15167488 bytes

spec:
Number of gates: 20100
Number of wires: 40200
Heap size: 13 Mb, 13824 Kb, 14155776 bytes

*/
void computerphile_sort()
{
	int len = 1000;

	IOCircuit circ = create_computerphile_sort_network
	(
		//porti_comparator_create,
		//portb_comparator_create,
		spec_comparator_create,
		len
	);

	Signal t_int = &lxc_signal_int;

	char name[20];
	{
		int i = -1;
		while(++i < len)
		{
			sprintf(name, "i%d", i);
			Wire w = get_or_create_wire(circ, name, t_int);
			LxcValue v = lxc_create_primitive_value(t_int);
			int* data = (int*) lxc_get_value(v);
			*data = rand();
			printf("i%d: %d\n", i, *data);
			lxc_drive_wire_value(NULL, 0, w, v);
			lxc_unreference_value(v);
		}
	}

	lxc_circuit_set_all_gate_enable(circ, true);
	printf("\n");
	//TODO wait circuit idle.


	{
		int i = -1;
		while(++i < len)
		{
			sprintf(name, "o%d", i);
			Wire w = get_or_create_wire(circ, name, t_int);
			LxcValue v = w->current_value;
			if(NULL != v)
			{
				int* data = (int*) lxc_get_value(v);
				printf("o%d: %d\n", i, *data);
			}
			else
			{
				printf("o%d: none\n", i);
			}
		}
	}

	printf("Number of gates: %d\n", hashmap_length(circ->gates));
	printf("Number of wires: %d\n", hashmap_length(circ->wires));

	linux_print_heap_size();

	//lxc_dbg_print_dot_graph(circ);

	/*
	{
		int i = -1;
		while(NULL != circ->wires[++i])
		{
			lxc_dbg_print_wire(circ->wires[i]);
			printf(" | ");
			lxc_dbg_print_value(circ->wires[i]->current_value);
			printf("\n");
		}
	}
	*/

	//lxc_destroy_circuit(circ);
}

int main__(int argc, char **argv)
{
	int elem = 10;

	int i = -1;
	int j;
	while(++i < elem)
	{
		j = -1;
		while(++j < i+1)
		{
			printf("%d ", triangle_nth(i, j));
		}
		printf("\n");
	}
}
/*
int main(int argc, char **argv)
{
	//register_option("", );
	register_option("computerphile_sort", computerphile_sort);


	char* ref = NULL;
	if(argc > 1)
	{
		ref = argv[1];
		int i=-1;
		while(NULL != OPTS[++i])
		{
			if(0 == strcmp(ref, OPTS[i]->name))
			{
				logxcontroll_init_environment();
				OPTS[i]->funct();
				logxcontroll_destroy_environment();
				return 0;
			}
		}
	}

	printf("Given usecase: \"%s\" not found.\n", ref);
	printf("Available usecases:\n");
	{
		int i = -1;
		while(NULL != OPTS[++i])
		{
			printf("==> \"%s\"\n", OPTS[i]->name);
		}
	}
	return 0;
}*/

