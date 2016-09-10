#include "posix/liblxc_posix.h"

int IN_ABS_PATH;
int IN_PATH;
int IN_ABS_FLAGS;
int IN_FLAGS;
int IN_ABS_MODE;
int IN_MODE;
int OUT_ABS_FD;
int OUT_FD;
int OUT_ABS_ERRNO;
int OUT_ERRNO;


static int fd_open_libop
(
	enum library_operation op,
	const char** errors,
	int max_length
);

static struct lxc_generic_portb_behavior fd_open_behavior =
{
	.base.library_operation = fd_open_libop,
};

struct fd_open_instance
{
	struct lxc_generic_portb_instance base;
	LxcValue fd;
	LxcValue last_errno;
};

static LxcValue* access_internal_variable(Gate instance, int index)
{
	struct fd_open_instance* gate =
		(struct fd_open_instance*) instance;

	if(index == OUT_ABS_FD)
	{
		return &(gate->fd);
	}
	else if(index == OUT_ABS_ERRNO)
	{
		return &(gate->last_errno);
	}
	return NULL;
}

static void fd_open_execute
(
	Gate instance,
	Signal type,
	int subtype,
	LxcValue value,
	uint index
)
{
	struct fd_open_instance* gate =
		(struct fd_open_instance*) instance;

	if(type == &lxc_signal_system)
	{
		struct lxc_system_event* sys =
			(struct lxc_system_event*) lxc_get_value(value);

		enum lxc_system_event_type type = sys->event_type;

		if(system_event_gate_disabled == type)
		{
			lxc_import_new_value(NULL, &(gate->last_errno));
			lxc_import_new_value(NULL, &(gate->fd));
			return;
		}
		else if(system_event_output_wire_added == type)
		{
			lxc_portb_republish_internal_value
			(
				instance,
				sys->signal,
				sys->subtype,
				sys->index,
				access_internal_variable
			);
			return;
		}
		return;
	}

	if(type == &lxc_signal_string || type == &lxc_signal_int)
	{
		LxcValue path = lxc_get_value_from_tokenport_array(gate->base.inputs,  IN_PATH);
		LxcValue flags = lxc_get_value_from_tokenport_array(gate->base.inputs, IN_FLAGS);
		bool mode_wired = gate->base.inputs[IN_FLAGS];

		LxcValue mode = mode_wired?lxc_get_value_from_tokenport_array(gate->base.inputs, IN_MODE):NULL;

		if
		(
			NULL != path
		&&
			NULL != flags
		&&
			NULL != mode
		&&
			(!mode_wired || NULL != mode)
		)
		{
			lxc_absorb_token(gate->base.inputs[IN_PATH]);
			lxc_absorb_token(gate->base.inputs[IN_FLAGS]);
			if(mode_wired)
			{
				lxc_absorb_token(gate->base.inputs[IN_MODE]);
			}

			char* p_path = *((char**)lxc_get_value(path));
			int p_flags = *((int*)lxc_get_value(flags));
			int p_mode = 0;

			if(mode_wired)
			{
				p_mode = *((int*)lxc_get_value(mode));
			}

			int fd = open(p_path, p_flags, p_mode);

			LxcValue o_fd = NULL;
			LxcValue o_errno = NULL;

			//error
			if(fd < 0)
			{
				int err = errno;
				o_errno = lxc_create_primitive_value(&lxc_signal_int);
				*((int*)lxc_get_value(o_errno)) = err;

				lxc_import_new_value(o_errno, &(gate->last_errno));
			}
			//success
			else
			{
				o_fd = lxc_create_primitive_value(&lxc_signal_int);
				*((int*)lxc_get_value(o_fd)) = fd;

				o_errno = lxc_integer_constant_value_0.value;

				lxc_import_new_value(o_fd, &(gate->fd));
				lxc_import_new_value(o_errno, &(gate->last_errno));
			}

			Wire out = gate->base.outputs[OUT_FD];

			if(NULL != o_fd && NULL != out)
			{
				lxc_drive_wire_value(instance, OUT_FD, out, o_fd);
			}

			out = gate->base.outputs[OUT_ERRNO];
			if(NULL != out)
			{
				lxc_drive_wire_value(instance, OUT_ERRNO, out, o_errno);
			}

			return;
		}
	}
}

static int fd_open_libop
(
	enum library_operation op,
	const char** errors,
	int max_length
)
{
	if(library_before_load == op)
	{
		lxc_init_from_prototype
		(
			(void*)	&fd_open_behavior,
			sizeof	(fd_open_behavior),

			(void*)	&lxc_generic_portb_propb_prototype,
			sizeof	(lxc_generic_portb_propb_prototype)
		);

		fd_open_behavior.base.library_operation = fd_open_libop;
		fd_open_behavior.base.gate_name = "FD Open";
		fd_open_behavior.base.paths = lxc_posix_path_file_descriptor;

		fd_open_behavior.instance_memory_size =
			sizeof(struct fd_open_instance);

		fd_open_behavior.base.execute = fd_open_execute;

		IN_ABS_PATH = lxc_port_unchecked_add_new_port
		(
			&(fd_open_behavior.output_ports),
			"path",
			&lxc_signal_string,
			0,
			&IN_PATH
		);

		IN_ABS_FLAGS = lxc_port_unchecked_add_new_port
		(
			&(fd_open_behavior.output_ports),
			"flags",
			&lxc_signal_int,
			0,
			&IN_FLAGS
		);

		IN_ABS_MODE = lxc_port_unchecked_add_new_port
		(
			&(fd_open_behavior.output_ports),
			"mode",
			&lxc_signal_int,
			0,
			&IN_MODE
		);

		OUT_ABS_FD = lxc_port_unchecked_add_new_port
		(
			&(fd_open_behavior.output_ports),
			"fd",
			&lxc_signal_int,
			0,
			&OUT_FD
		);

		OUT_ABS_ERRNO = lxc_port_unchecked_add_new_port
		(
			&(fd_open_behavior.output_ports),
			"errno",
			&lxc_signal_int,
			0,
			&OUT_ERRNO
		);

	}
	return 0;
}


void* produce_fd_open()
{
	return (void*) &fd_open_behavior;
}
