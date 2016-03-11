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

int lxc_reference_primitive_value(LxcValue asdf)
{
	struct lxc_primitive_value* val = (struct lxc_primitive_value*) asdf;
	if
	(
			NULL != val
		&&
			NULL != val->base.operations
		&&
			NULL != val->base.operations->reference
	)
	{
		return __sync_fetch_and_add(&(val->base.refcount), 1);
	}

	return 1;
}

int lxc_unreference_primitive_value(LxcValue asdf)
{
	struct lxc_primitive_value* val = (struct lxc_primitive_value*) asdf;
	if
	(
			NULL != val
		&&
			NULL != val->base.operations
		&&
			NULL != val->base.operations->unreference
	)
	{
		return __sync_fetch_and_sub(&(val->base.refcount), 1);
	}

	return 1;
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
	.reference = lxc_reference_primitive_value,
	.unreference = lxc_unreference_primitive_value,
	.data_address = lxc_data_address_primitive_value,
};

const struct lxc_value_operation primitive_constant_value_operations =
{
	.free = NULL,
	.clone = lxc_clone_primitive_value,
	.size = lxc_size_primitive_value,
	.reference = NULL,
	.unreference = NULL,
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

const struct lxc_signal_type lxc_signal_pcm =
{
	.name = "pcm",
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

static int library_load(enum library_operation op, char*** errors)
{
	return 0;
}

static const struct lxc_gate_behavior placeholder;

static struct detailed_gate_entry detailed_cast_to =
{
	.behavior = &placeholder,
	.generic_name = "cast to",
	.paths = (char**[]){(char*[]){"Built in", "Type", NULL},NULL},
};

static struct detailed_gate_entry detailed_const =
{
	.behavior = &placeholder,
	.generic_name = "const",
	.paths = (char**[]){(char*[]){"Built in", "Value propagation", NULL},NULL},
};

const struct loadable_library logxcontroll_loadable_library_builtin =
{
	.library_operation = library_load,
	.gates = (struct detailed_gate_entry*[])
	{
		&detailed_cast_to,
		&detailed_const,



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

		&lxc_signal_pcm,

		NULL
	},

	.constants = (struct lxc_constant_value*[])
	{
		&lxc_bool_constant_value_false,
		&lxc_bool_constant_value_true,

		NULL
	}
};
