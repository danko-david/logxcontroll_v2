/*
 * builtin.c
 *
 *  Created on: 2016.02.27.
 *      Author: szupervigyor
 */

#include "core/logxcontroll.h"

LxcValue lxc_create_primitive_value(Signal type)
{
	struct lxc_primitive_value* ret = malloc(sizeof(struct lxc_primitive_value));
	ret->base.type = type;
	ret->base.refcount = 0;
	return (LxcValue) ret;
}

void lxc_free_primitive_value(LxcValue value)
{
	free(value);
}

LxcValue lxc_clone_primitive_value(LxcValue value)
{
	struct lxc_primitive_value* ret = malloc(sizeof(struct lxc_primitive_value));

	memcpy(ret, value, sizeof(struct lxc_primitive_value));

	ret->base.operations = &primitive_variable_value_operations;

	return (LxcValue) ret;
}

size_t lxc_size_primitive_value(/*LxcValue value*/)
{
	return sizeof(double);
}

static int lxc_ref_diff_primitive_value(LxcValue asdf, int n)
{
	struct lxc_primitive_value* val = (struct lxc_primitive_value*) asdf;
	return __sync_fetch_and_add(&(val->base.refcount), n);
}



void* lxc_data_address_primitive_value(LxcValue val)
{
	return &(((struct lxc_primitive_value*) val)->char_value);
}

const struct lxc_value_operation primitive_variable_value_operations =
{
	.free = lxc_free_primitive_value,
	.clone = lxc_clone_primitive_value,
	.size = lxc_size_primitive_value,
	.ref_diff = lxc_ref_diff_primitive_value,
	.data_address = lxc_data_address_primitive_value,
};

const struct lxc_value_operation primitive_constant_value_operations =
{
	.free = NULL,
	.clone = lxc_clone_primitive_value,
	.size = lxc_size_primitive_value,
	.ref_diff = NULL,
	.data_address = lxc_data_address_primitive_value,
};

const struct lxc_signal_type lxc_signal_bool =
{
	.name = "bool",
};

const struct lxc_signal_type lxc_signal_byte =
{
	.name = "byte",
};

const struct lxc_signal_type lxc_signal_short =
{
	.name = "short",
};

const struct lxc_signal_type lxc_signal_int =
{
	.name = "int",
};

const struct lxc_signal_type lxc_signal_float =
{
	.name = "float",
};

const struct lxc_signal_type lxc_signal_long =
{
	.name = "long",
};

const struct lxc_signal_type lxc_signal_double =
{
	.name = "double",
};

const struct lxc_signal_type lxc_signal_data =
{
	.name = "data",
	//.equals =
};

static const struct lxc_primitive_value lxc_bool_value_true =
{
	.base.type = &lxc_signal_bool,
	.base.refcount = 1024,
	.base.operations = &primitive_constant_value_operations,
	.char_value = ~0,
};

static const struct lxc_primitive_value lxc_bool_value_false =
{
	.base.type = &lxc_signal_bool,
	.base.refcount = 1024,
	.base.operations = &primitive_constant_value_operations,
	.char_value = 0,
};

const struct lxc_constant_value lxc_bool_constant_value_true =
{
	.value = &lxc_bool_value_true,
	.name = "true"
};

const struct lxc_constant_value lxc_bool_constant_value_false =
{
	.value = &lxc_bool_value_false,
	.name = "false"
};

/************************** Built in libraries ********************************/

const char*** lxc_built_in_path_type =	(char**[])
									{
										(char*[])
										{
											"Built in",
											"Type",
											NULL
										},
										NULL
									};

const char*** lxc_built_in_path_value_propagation =	(char**[])
												{
													(char*[])
													{
														"Built in",
														"Value propagation",
														NULL
													},
													NULL
												};

static int library_load(enum library_operation op, char*** errors)
{
	if(library_before_load == op)
	{
		lxc_builtin_cast_init_before_load();
	}
	else if(library_after_loaded == op)
	{



	}


	return 0;
}

/*static struct detailed_gate_entry detailed_const =
{
	.behavior = &placeholder,
	.generic_name = "const",
	.paths = lxc_built_in_path_value_propagation,
};
*/

const struct lxc_loadable_library logxcontroll_loadable_library_builtin =
{
	.library_operation = library_load,
	.gates = (struct detailed_gate_entry*[])
	{
		//&lxc_built_in_gate_cast,
		//&detailed_const,



		NULL
	},
	.signals = (Signal[])
	{
		&lxc_signal_bool,
		&lxc_signal_byte,
		&lxc_signal_double,
		&lxc_signal_float,
		&lxc_signal_int,
		&lxc_signal_long,
		&lxc_signal_short,

		&lxc_signal_data,

		NULL
	},

	.constants = (struct lxc_constant_value*[])
	{
		&lxc_bool_constant_value_false,
		&lxc_bool_constant_value_true,

		NULL
	}
};

