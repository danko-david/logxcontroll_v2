/*
 * socket_create.c
 *
 * Input ports:
 * 		AF_ 	(int) - address family constant
 * 		SOCK_	(int) - socket type
 *		PF_		(int) - protocol family
 * 		new		(pluse)	- create a new socket
 *
 * Output ports:
 * 		socket fd 	(int) - socket fd
 * 		errno		(int) - error number
 *
 *  Created on: 2016.03.30.
 *      Author: szupervigyor
 */

#include "liblxc_posix.h"

static int IN_AF;
static int IN_SOCK;
static int IN_PF;
static int IN_new;

static int OUT_socket_fd;
static int OUT_errno;

static int socket_create_libop
(
	enum library_operation op,
	const char** errors,
	int max_length
);

static struct lxc_generic_portb_behavior posix_socket_create =
{
	.base.library_operation = socket_create_libop,
};

struct lxc_posix_socket_create_instance
{
	struct lxc_generic_portb_instance base;
	LxcValue out_socket_fd;
	LxcValue out_errno;
};

static LxcValue* access_internal_variable(Gate i, int wire_abs)
{
	struct lxc_posix_socket_create_instance* gate =
		(struct lxc_posix_socket_create_instance*) i;

	if(wire_abs == OUT_socket_fd)
	{
		return &(gate->out_socket_fd);
	}
	else if(wire_abs == OUT_errno)
	{
		return &(gate->out_errno);
	}

	return NULL;
}

static inline int get_input_value(struct lxc_posix_socket_create_instance* gate, int abs_index)
{
	Wire w = gate->base.inputs[abs_index];
	if(NULL == w)
	{
		return -1;
	}

	LxcValue val = lxc_get_wire_value(w);
	if(NULL == val)
	{
		return -1;
	}

	int* num = (int*) lxc_get_value(val);
	return *num;
}

static int do_operation(struct lxc_posix_socket_create_instance* gate)
{
	int af = get_input_value(gate, IN_AF);
	int sock = get_input_value(gate, IN_SOCK);
	int pf = get_input_value(gate, IN_PF);

	if(-1 == af || -1 == sock || -1 == pf)
	{
		return LXC_ERROR_NOTHING_CHANGED;
	}

	int sock_out = socket(af, sock, pf);

	if(sock_out < 0)
	{
		return errno;
	}

	lxc_wipe_value(&(gate->out_socket_fd));
	LxcValue n = lxc_create_generic_value(&lxc_signal_int, sizeof(int));
	lxc_reference_value(n);
	int* val = lxc_get_value(n);
	*val = sock_out;
	gate->out_socket_fd = n;
	lxc_drive_wire_value
	(
		(Gate) gate,
		0,
		gate->base.outputs[OUT_socket_fd],
		n
	);

	return 0;
}

static void operation(struct lxc_posix_socket_create_instance* gate)
{
	int err = do_operation(gate);
	if(LXC_ERROR_NOTHING_CHANGED != err)
	{
		//if errno is the same as previous
		if(NULL != gate->out_errno)
		{
			if(err == *((int*)lxc_get_value(gate->out_errno)))
			{
				return;
			}
		}

		lxc_wipe_value(&(gate->out_errno));
		LxcValue n = lxc_create_generic_value(&lxc_signal_int, sizeof(int));
		lxc_reference_value(n);
		int* val = lxc_get_value(n);
		*val = err;
		gate->out_errno = n;
		lxc_drive_wire_value
		(
			(Gate) gate,
			0,
			gate->base.outputs[OUT_errno],
			n
		);
	}
}

static void socket_create_execute(Gate instance, Signal type, LxcValue value, uint index)
{
	struct lxc_posix_socket_create_instance* gate =
		(struct lxc_posix_socket_create_instance*) instance;

	if(type == &lxc_signal_system)
	{
		struct lxc_system_event* sys =
			(struct lxc_system_event*) lxc_get_value(value);

		enum lxc_system_event_type type = sys->event_type;

		if(system_event_gate_enabled == type)
		{
			operation(gate);
		}
		else if(system_event_output_wire_added == type)
		{
			lxc_portb_republish_internal_value
			(
				instance,
				sys->signal,
				sys->index,
				access_internal_variable
			);
		}
	}

	operation(gate);
}


static int socket_create_libop
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
			(void*)	&posix_socket_create,
			sizeof	(posix_socket_create),

			(void*)	&lxc_generic_portb_prototype,
			sizeof	(lxc_generic_portb_prototype)
		);

		posix_socket_create.base.library_operation = socket_create_libop;
		posix_socket_create.base.gate_name = "socket create";
		posix_socket_create.base.paths = lxc_posix_path_socket;

		posix_socket_create.instance_memory_size =
			sizeof(struct lxc_posix_socket_create_instance);

		posix_socket_create.base.execute = socket_create_execute;


		IN_AF = lxc_port_unchecked_add_new_port
		(
			&(posix_socket_create.input_ports),
			"AF_",
			&lxc_signal_int,
			true
		);

		IN_SOCK = lxc_port_unchecked_add_new_port
		(
			&(posix_socket_create.input_ports),
			"SOCK_",
			&lxc_signal_int,
			true
		);

		IN_PF = lxc_port_unchecked_add_new_port
		(
			&(posix_socket_create.input_ports),
			"PF_",
			&lxc_signal_int,
			true
		);

		IN_new = lxc_port_unchecked_add_new_port
		(
			&(posix_socket_create.input_ports),
			"new",
			&lxc_signal_pulse,
			true
		);

		OUT_socket_fd = lxc_port_unchecked_add_new_port
		(
			&(posix_socket_create.output_ports),
			"socket fd",
			&lxc_signal_int,
			false
		);


		OUT_errno = lxc_port_unchecked_add_new_port
		(
			&(posix_socket_create.output_ports),
			"errno",
			&lxc_signal_int,
			false
		);

	}
	return 0;
}


void* produce_posix_socket_create()
{
	return (void*) &posix_socket_create;
}
