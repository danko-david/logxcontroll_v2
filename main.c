
#include <stdio.h>
#include <unistd.h>
#include "core/logxcontroll.h"

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
/*
Gate create_nand()
{
	return logic_nand.base.create(&(logic_nand));
}

Wire bool_wire()
{
	return lxc_create_wire(&lxc_signal_bool);
}
*/
/*

struct val
{
	int length;
	const char txt[20];
};

const struct val my_val =
{
	.txt = "Az én kis hosszú szövegem",
	.length = sizeof(my_val.txt),
};
*/

struct dyn_size
{
	size_t length;
	char data[];
};

/***/

void test_switch()
{
	//Gate sw = builtin_gate_switch.create(&builtin_gate_switch);
/*
	struct lxc_port_manager_factory* fact = lxc_lazy_new_port_manager_factory();

	struct port_def a;
	struct port_def b;
	struct port_def c;

	a.type = &lxc_signal_bool;
	a.sensitive = true;
	a.name = "enable";

	b.type = &lxc_signal_double;
	b.sensitive = false;
	b.name = "value";

	c.type = &lxc_signal_bool;
	c.sensitive = false;
	c.name = "byte";

	lxc_lazy_unchecked_add_new_port(fact, &a);
	lxc_lazy_unchecked_add_new_port(fact, &b);
	lxc_lazy_unchecked_add_new_port(fact, &c);

	printf("a port index: %d\r\n", lxc_lazy_get_absindex_by_name(fact, "enable"));
	printf("b port index: %d\r\n", lxc_lazy_get_absindex_by_name(fact, "value"));
	printf("c port index: %d\r\n", lxc_lazy_get_absindex_by_name(fact, "byte"));*/
}

#include "core/logxcontroll.h"
//#include "module/logic/lxc_bool_gates.h"

/*
void test_path()
{
	logxcontroll_init_environment();
	printf("lib initialized\n");
	char** errors = NULL;
	//lxc_load_library(&logxcontroll_loadable_library_bool, &errors);
	dbg_print_messages(errors);
	lxc_new_instance_by_name("");
	dbg_print_library_tree(true);

	Gate nand = lxc_new_instance_by_name("nand");
	if(NULL != nand)
		printf("instance name: %s\n",nand->behavior->get_gate_name(nand));

}
*/

void test_cast()
{
	Gate cast = lxc_new_instance_by_name("cast to");

	char*** arr = malloc(sizeof(void*));
	arr[0] = NULL;

	array_pnt_append_element(arr, malloc(20));
	array_pnt_append_element(arr, malloc(20));
	array_pnt_append_element(arr, malloc(20));
	array_pnt_append_element(arr, malloc(20));

	int ret = cast->behavior->enumerate_properties(cast, *arr, 4);

	printf("ret: %d\n", ret);

	char** a = *arr;

	for(int i=0;NULL != a[i];++i)
		printf("%s\n", a[i]);
}

void init_env()
{
	logxcontroll_init_environment();
	printf("library initialized\n");
	dbg_print_library_tree(true);
}


int main(/*int dfgsdfg, char** argv*/)
{
	if(SIG_ERR == signal(SIGSEGV, gnu_libc_print_stack_trace))
	{
		printf("Can't register print_stack_trace crash handler\n!");
	}

	init_env();

	test_cast();
	//test_path();

	exit(0);
/*	struct dyn_size* st = malloc(sizeof(struct dyn_size)+20);

	strcpy(st->data, "cucc");

	printf("%s", &st->data[0]);



	exit(0);

*/
/*	Wire in = bool_wire();
	Wire out = bool_wire();

	Wire ac = bool_wire();
	Wire cd =
				//ac;
				bool_wire();

	Wire bd = bool_wire();

	Gate A = create_nand();
	Gate B = create_nand();
	Gate C = create_nand();
	Gate D = create_nand();

	lxc_wire_gate_input(&lxc_signal_bool, in, A, 0);
	lxc_wire_gate_input(&lxc_signal_bool, in, B, 0);

	lxc_wire_gate_output(&lxc_signal_bool, ac, A , 0);
	lxc_wire_gate_input(&lxc_signal_bool, ac, C , 0);

	lxc_wire_gate_output(&lxc_signal_bool, cd, C, 0);
	lxc_wire_gate_input(&lxc_signal_bool, cd, D, 0);

	lxc_wire_gate_output(&lxc_signal_bool, bd, B, 0);

	lxc_wire_gate_input(&lxc_signal_bool, bd, D, 1);

	Gate dbg = logic_print.base.create(&(logic_print.base));

	lxc_wire_gate_output(&lxc_signal_bool, out, D, 0);

	lxc_wire_gate_input(&lxc_signal_bool, out, dbg, 0);

	printf("everyting is wired\n");
	A->enabled = ~0;
	B->enabled = ~0;
	C->enabled = ~0;
	D->enabled = ~0;
	dbg->enabled = ~0;

	lxc_drive_wire_value(NULL, 0, in, &(lxc_bool_value_false));
	lxc_drive_wire_value(NULL, 0, in, &(lxc_bool_value_true));

	linux_print_heap_size();*/
	return 0;
}

