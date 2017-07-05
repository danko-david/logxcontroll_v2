
#include "core/logxcontroll.h"

#define OPERAND_A 10
#define OPERAND_B 35
#define OPERAND_RESULT 45

static void func_add_num(int** add2, LxcValue* sum)
{
	sum[0] = lxc_create_primitive_value(&lxc_signal_int);
	int* val = lxc_get_value(sum[0]);
	*val = *add2[0] + *add2[1];
	#ifdef NP_ASSERT
		NP_ASSERT_EQUAL(*val, OPERAND_RESULT);
	#endif
}

static void func_print_num(int** num1, LxcValue* _)
{
	printf("Gate: print_num: %d\n", *num1[0]);
	#ifdef NP_ASSERT
		NP_ASSERT_EQUAL(*num1[0], OPERAND_RESULT);
	#endif
}

static void test_functional_wrap()
{
	Signal s_int = &lxc_signal_int;

	lxc_register_gate
	(
		(Behavior) lxc_generic_shorthand_func_wrap
		(
			"add",
			func_add_num,
			"a", s_int, 0,
			"b", s_int, 0,
			NULL,
			"out", s_int, 0,
			NULL
		)
	);

	lxc_register_gate
	(
		(Behavior) lxc_generic_shorthand_func_wrap
		(
			"print_num",
			func_print_num,
			"val", s_int, 0,
			NULL,
			NULL
		)
	);

	IOCircuit circ = lxc_circuit_create();

	Wire a = lxc_circuit_get_or_create_wire(circ, "a", s_int);
	Wire b = lxc_circuit_get_or_create_wire(circ, "b", s_int);

	Wire sum = lxc_circuit_get_or_create_wire(circ, "sum", s_int);

	{
		Gate add = lxc_create_gate_by_name("add");
		lxc_gate_set_refdes(add, "add");
		lxc_circuit_add_gate(circ, add);

		lxc_wire_gate_input(s_int, 0, a, add, 0);
		lxc_wire_gate_input(s_int, 0, b, add, 1);

		lxc_wire_gate_output(s_int, 0, sum, add, 0);
	}

	{
		Gate print = lxc_create_gate_by_name("print_num");
		lxc_gate_set_refdes(print, "print");
		lxc_circuit_add_gate(circ, print);

		lxc_wire_gate_input(s_int, 0, sum, print, 0);
	}

	lxc_circuit_set_all_gate_enable(circ, true);

	LxcValue v_a = lxc_create_primitive_value(s_int);
	int* val_a = lxc_get_value(v_a);

	LxcValue v_b = lxc_create_primitive_value(s_int);
	int* val_b = lxc_get_value(v_b);

	*val_a = OPERAND_A;
	*val_b = OPERAND_B;

	lxc_dbg_print_dot_graph(circ);

	lxc_drive_wire_value(NULL, 0, a, v_a);
	lxc_drive_wire_value(NULL, 0, b, v_b);

	lxc_unreference_value(v_a);
	lxc_unreference_value(v_b);

	lxc_circuit_set_all_gate_enable(circ, false);

	lxc_circuit_destroy(circ);
}

void functional_wrap(int argc, char **argv, int start_from)
{
	test_functional_wrap();
}
