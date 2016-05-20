/*
 * fd_io.c
 *
 *	Defines the file descriptor read/write operations:
 *
 *	input ports:
 *		(data) in
 *		(bool) read
 *
 *	output ports:
 *		(data) out
 *		(int) errno
 *
 *	properties:
 *		read_mode: (auto, rise, fall, toggle)
 *		buffer_size_bytes: 10240 (default)
 *
 *  Created on: 2016.03.17.
 *      Author: szupervigyor
 */

#include "liblxc_posix.h"
/*

struct lxc_posix_fd_io_instance
{
	struct lxc_generic_portb_propb_behavior base;
	char* buffer;
	size_t buff_length;
	pthread_t reader;
};

static void* access_prop_value(Gate instance, const char* property)
{
	//TODO
	return NULL;
}

static int validate_read_mode
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
	return 0;
}

static int validate_buffer_size
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
	return 0;
}

static void fd_io_execute(Gate instance, Signal type, LxcValue value, uint index)
{


}

static struct lxc_generic_portb_propb_behavior fd_io_behavior;

static void libop(enum library_operation op)
{
	if(library_before_load == op)
	{
		lxc_init_from_prototype
		(
			&fd_io_behavior,
			sizeof(fd_io_behavior),

			&lxc_generic_portb_propb_prototype,
			sizeof(lxc_generic_portb_propb_prototype)
		);

		//register ports, properties,
		fd_io_behavior.base.gate_name = "fd io";
		fd_io_behavior.base.instance_memory_size = sizeof(struct lxc_posix_fd_io_instance);
		fd_io_behavior.base.base.execute = fd_io_execute;
		fd_io_behavior.properties.access_property = access_prop_value;

 		//.instance_init
 		//.instance_destroy
		//.base.gatectl
		//.properties.notify_property_changed
		lxc_port_unchecked_add_new_port
		(
			&(fd_io_behavior.base.input_ports),
			"in",
			&lxc_signal_data,
			true
		);

		lxc_port_unchecked_add_new_port
		(
			&(fd_io_behavior.base.input_ports),
			"read",
			&lxc_signal_bool,
			true
		);

		lxc_port_unchecked_add_new_port
		(
			&(fd_io_behavior.base.output_ports),
			"out",
			&lxc_signal_data,
			false
		);

		lxc_port_unchecked_add_new_port
		(
			&(fd_io_behavior.base.output_ports),
			"errno",
			&lxc_signal_int,
			false
		);

		lxc_add_property
		(
			&(fd_io_behavior.properties),
			"read_mode",
			"Read triggering",
			"Specifies, which kind of event performs the reading from the file descriptor.",
			"",
			validate_read_mode
		);

		lxc_add_property
		(
			&(fd_io_behavior.properties),
			"buffer_size_bytes",
			"Receive buffer size (bytes)",
			"Specifies the size of the incoming data size (in bytes) can be readed in a single read cycle.",
			"",
			 validate_buffer_size
		);
	}
}
*/
