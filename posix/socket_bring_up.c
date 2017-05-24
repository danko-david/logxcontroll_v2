/*
 * socket_bring_up.c
 *
 *	Properties:
 *		mode	- connect/listen (default: connect)
 *		backlog	- used only in listen mode (maximum pending connections before
 *					connection to this host is refused) (default: 128 as in
 *						/usr/src/linux/net/ipv4/af_inet.c SOMAXCONN is defined)
 *
 *	Input ports:
 *		socket fd	int		 - created socket's file descriptor
 *		local		sockaddr - local address descriptor
 *		remote		sockaddr - remote address descriptor
 *
 *	Output ports:
 *		fd		(int) - file descriptor
 *		errno	(int) - 0 on no error, others means error.
 *
 *  Created on: 2016.03.05.
 *      Author: szupervigyor
 */

#include "liblxc_posix.h"

static int bring_up_libop
(
	enum library_operation op,
	const char** errors,
	int max_length
);

static struct lxc_generic_portb_propb_behavior posix_socket_bring_up =
{
	.base.base.library_operation = bring_up_libop,
};

static const int MODE_CONNECT = 0;
static const int MODE_LISTEN = 1;

static int IN_ABS_SOCKET_FD;
static int IN_ABS_LOCAL;
static int IN_ABS_REMOTE;

static int OUT_ABS_FD;
static int OUT_ABS_ERRNO;

struct lxc_posix_bring_up_instance
{
	struct lxc_generic_portb_instance base;
	int mode;
	int backlog;
	LxcValue socket_fd;
	LxcValue local;
	LxcValue remote;

	LxcValue out_fd;
	LxcValue out_errno;
};

static LxcValue* access_internal_variable(Gate g, int index)
{
	struct lxc_posix_bring_up_instance* gate =
			(struct lxc_posix_bring_up_instance*) g;

	if(index == OUT_ABS_FD)
	{
		return &(gate->out_fd);
	}
	else if(index == OUT_ABS_ERRNO)
	{
		return &(gate->out_errno);
	}

	return NULL;
}

static void pass_through_fd(struct lxc_posix_bring_up_instance* gate)
{
	Wire out = gate->base.outputs[OUT_ABS_FD];
	if(NULL != out)
	{
		lxc_drive_wire_value((Gate) gate, OUT_ABS_FD, out, gate->socket_fd);
	}
}

static void publish_errno(struct lxc_posix_bring_up_instance* gate, int error_number)
{
	if(NULL != gate->out_errno)
	{
		int* num = (int*)lxc_get_value(gate->out_errno);
		if(*num == error_number)
		{
			return;
		}
	}

	LxcValue val = lxc_create_generic_value(&lxc_signal_int, sizeof(int));
	lxc_reference_value(val);
	gate->out_errno = val;
	int* err = (int*) lxc_get_value(val);
	*err = error_number;
	Wire out = gate->base.outputs[OUT_ABS_ERRNO];
	if(NULL == out)
	{
		return;
	}

	lxc_drive_wire_value((Gate) gate, OUT_ABS_ERRNO, out, val);
}

static int do_bring_up(struct lxc_posix_bring_up_instance* gate)
{
	int mode = gate->mode;

	printf("do_bring_up: socket_fd: %p, remove: %p, local: %p\n", gate->socket_fd, gate->remote, gate->local);
	fsync(1);

	if(NULL == gate->socket_fd)
	{
		return LXC_ERROR_NOTHING_CHANGED;
	}

	int sock_fd = *((int*)lxc_get_value(gate->socket_fd));

	if(sock_fd < 0)
	{
		return EBADF;
	}

	if(MODE_CONNECT == mode)
	{
		if(NULL == gate->remote)
		{
			return LXC_ERROR_NOTHING_CHANGED;
		}

		/*
		 * only remote required
		 * but if local available we call bind
		 */
		LxcValue lv = gate->local;
		if(NULL != lv)
		{
			const struct sockaddr* local_address =
				(const struct sockaddr*) lxc_get_value(lv);

			int size = lxc_value_size(lv);
			int ret = bind(sock_fd, local_address, size);
			if(0 != ret)
			{
				return errno;
			}
		}

		struct sockaddr* remote_addr = (struct sockaddr*) lxc_get_value(gate->remote);
		int size = lxc_value_size(gate->remote);

		int ret = connect(sock_fd, remote_addr, size);
		if(0 != ret)
		{
			return errno;
		}

		return SUCCESS;//real success
	}
	else if(MODE_LISTEN == mode)
	{
		//only local required, if remote given we refuse listen
		if(NULL != gate->remote)
		{
			return LXC_ERROR_BAD_CALL;
		}

		if(NULL == gate->local)
		{
			return LXC_ERROR_NOTHING_CHANGED;
		}

		struct sockaddr* local = lxc_get_value(gate->local);
		int size = lxc_value_size(gate->local);

		int ret = bind(sock_fd, local, size);
		if(0 != ret)
		{
			return errno;
		}

		ret = listen(sock_fd, gate->backlog);
		if(0 != ret)
		{
			return errno;
		}

		return SUCCESS;
	}
	else
	{
		return LXC_ERROR_ENTITY_OUT_OF_RANGE;
	}
}

static void bring_up(struct lxc_posix_bring_up_instance* gate)
{
	int ret = do_bring_up(gate);
	if(0 == ret)
	{
		pass_through_fd(gate);
	}

	if(ret == LXC_ERROR_NOTHING_CHANGED)
	{
		return;
	}

	publish_errno(gate, ret);
}

static void bring_down(struct lxc_posix_bring_up_instance* gate)
{
	lxc_wipe_value(&(gate->out_fd));
	lxc_wipe_value(&(gate->out_errno));
}

static void bring_again(struct lxc_posix_bring_up_instance* gate)
{
	bring_down(gate);
	bring_up(gate);
}

static void bring_up_execute(Gate instance, Signal type, int subtype, LxcValue value, uint index)
{
	struct lxc_posix_bring_up_instance* gate =
		(struct lxc_posix_bring_up_instance*) instance;

	if(type == &lxc_signal_system)
	{
		struct lxc_system_event* sys =
			(struct lxc_system_event*) lxc_get_value(value);

		enum lxc_system_event_type type = sys->event_type;
		if(system_event_gate_disabled == type)
		{
			bring_down(gate);
		}
		else if(system_event_gate_enabled == type)
		{
			bring_up(gate);
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
		}
		else if(system_event_property_modified == type)
		{
			bring_again(gate);
		}

		return;
	}

	int abs = lxc_portb_get_absindex(&(gate->base), DIRECTION_IN, type, subtype, index);
	setbuf(stdout, NULL);
	printf("SOCKETR_BRING_UP_ABS_INDEX: %d\n", abs);
	fsync(1);

	bool again = false;

	//TODO read and absorb
	if(abs == IN_ABS_SOCKET_FD)
	{
		printf("IMPORT_FD: %p\n", value);
		fsync(1);
		again = lxc_import_new_value(value, &(gate->socket_fd));
	}
	else if(abs == IN_ABS_LOCAL)
	{
		printf("IMPORT_LOCAL: %p\n", value);
		fsync(1);
		again = lxc_import_new_value(value, &(gate->local));
	}
	else if(abs == IN_ABS_REMOTE)
	{
		printf("IMPORT_REMOTE: %p\n", value);
		fsync(1);
		again = lxc_import_new_value(value, &(gate->remote));
	}

	if(again)
	{
		bring_again(gate);
	}
}

static void* bring_up_access_property(Gate instance, const char* name)
{
	if(0 == strcmp(name, "mode"))
	{
		return &(((struct lxc_posix_bring_up_instance*) instance)->mode);
	}
	else if(0 == strcmp(name, "backlog"))
	{
		return &(((struct lxc_posix_bring_up_instance*) instance)->backlog);
	}

	return NULL;
}

static int mode_validation
(
	Gate instance,
	bool direction,
	void* addr,
	const char* name,
	const char* value,
	char* ret,
	int max_length
)
{
	if(DIRECTION_IN == direction)
	{
		if(NULL == value)
		{
			*((int*)addr) = MODE_CONNECT;
			return 0;
		}

		else if(0 == strcmp(value, "connect"))
		{
			*((int*)addr) = MODE_CONNECT;
			return 0;
		}
		else if(0 == strcmp(value, "listen"))
		{
			*((int*)addr) = MODE_LISTEN;
			return 0;
		}
		else
		{
			return safe_strcpy(ret, max_length, "Mode can be only \"connect\" or \"listen\".");
		}
	}
	else
	{
		if(*((int*)addr) == MODE_CONNECT)
		{
			return safe_strcpy(ret, max_length, "connect");
		}
		else if(*((int*)addr) == MODE_LISTEN)
		{
			return safe_strcpy(ret, max_length, "listen");
		}

		return safe_strcpy(ret, max_length, "not_specified");
	}
}

static int backlog_validation
(
	Gate instance,
	bool direction,
	void* addr,
	const char* name,
	const char* value,
	char* ret,
	int max_length
)
{
	if(DIRECTION_IN == direction)
	{
		if(NULL == value)
		{
			*((int*)addr) = 128;
			return 0;
		}

		int tar = strtol(value , NULL, 10);
		if(tar < 1 || tar > 65536)
		{
			return safe_strcpy(ret, max_length, "backlog must be between 1 and 65535");
		}

		*((int*)addr) = tar;
		return 0;
	}
	else
	{
		snprintf(ret, max_length, "%d", *((int*)addr));
		return 0;
	}
}

static int bring_up_libop
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
			(void*) &posix_socket_bring_up,
			sizeof(posix_socket_bring_up),

			(void*) &lxc_generic_portb_propb_prototype,
			sizeof(lxc_generic_portb_propb_prototype)
		);

		posix_socket_bring_up.base.base.library_operation = bring_up_libop;
		posix_socket_bring_up.base.base.gate_name = "socket bring up";
		posix_socket_bring_up.base.base.paths = lxc_posix_path_socket;

		posix_socket_bring_up.base.instance_memory_size =
					sizeof(struct lxc_posix_bring_up_instance);

		posix_socket_bring_up.base.base.execute = bring_up_execute;

		posix_socket_bring_up.properties.access_property =
				bring_up_access_property;

		lxc_port_init_port_manager_factory(&(posix_socket_bring_up.base.input_ports));
		lxc_port_init_port_manager_factory(&(posix_socket_bring_up.base.output_ports));

		IN_ABS_SOCKET_FD = lxc_port_unchecked_add_new_port
		(
			&(posix_socket_bring_up.base.input_ports),
			"socket fd",
			&lxc_signal_int,
			0,
			NULL
		);

		IN_ABS_LOCAL = lxc_port_unchecked_add_new_port
		(
			&(posix_socket_bring_up.base.input_ports),
			"local",
			&lxc_posix_sockaddr,
			0,
			NULL
		);

		IN_ABS_REMOTE = lxc_port_unchecked_add_new_port
		(
			&(posix_socket_bring_up.base.input_ports),
			"remote",
			&lxc_posix_sockaddr,
			0,
			NULL
		);

		OUT_ABS_FD = lxc_port_unchecked_add_new_port
		(
			&(posix_socket_bring_up.base.output_ports),
			"fd",
			&lxc_signal_int,
			0,
			NULL
		);

		OUT_ABS_ERRNO = lxc_port_unchecked_add_new_port
		(
			&(posix_socket_bring_up.base.output_ports),
			"errno",
			&lxc_signal_int,
			0,
			NULL
		);

		lxc_add_property
		(
			&(posix_socket_bring_up.properties),
			"mode",
			"Connection Mode",
			"Specify what to do with addresses, connect to remote or listen on local address.",
			"connect",
			mode_validation
		);

		lxc_add_property
		(
			&(posix_socket_bring_up.properties),
			"backlog",
			"Backlog used for listen mode",
			"Specified the maximal number of pending connections before refusing remote host's connect's.",
			"128",
			backlog_validation
		);
	}

	return 0;
}

void* produce_posix_socket_bring_up()
{
	return &posix_socket_bring_up;
}
