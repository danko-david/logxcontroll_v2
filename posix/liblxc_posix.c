/*
 * liblxc_posix.c
 *
 *  Created on: 2016.03.05.
 *      Author: szupervigyor
 */
#include "liblxc_posix.h"

//TODO
static bool sockaddr_equals(LxcValue val1, LxcValue val2)
{
	int size1 = lxc_value_size(val1);
	int size2 = lxc_value_size(val2);

	if(0 == size1 || 0 == size2 || size1 != size2)
	{
		return false;
	}

	return 0 == memcmp(lxc_get_value(val1), lxc_get_value(val2), size1);
}

const struct lxc_signal_type lxc_posix_sockaddr =
{
	.name = "sockaddr",
	.equals = sockaddr_equals,

	//TODO cast_to
};

const char*** lxc_posix_path_file_descriptor =
(const char**[])
{
	(const char*[])
	{
		"Communication", "Networking", "POSIX", "File Descriptor", NULL
	},
	NULL
};

const char*** lxc_posix_path_socket =
(const char**[])
{
	(const char*[])
	{
		"Communication", "Networking", "POSIX", "Socket", NULL
	},
	NULL
};

static void add_gate(void* gate)
{
	array_pnt_append_element
	(
		(void***)&(POSIX_LIB_STRUCT_NAME.gates),
		gate
	);
}

int posix_libop(enum library_operation op, const char** error, int max_length)
{
	if(library_before_load == op)
	{
		add_gate(produce_posix_socket_create());
		add_gate(produce_posix_socketaddress_create());
		add_gate(produce_posix_socket_bring_up());
		/*add_gate(produce_posix_socket_accept());
		add_gate(produce_posix_fd_io());
		add_gate(produce_posix_serial());*/
	}

	int i;
	for(i=0;NULL != POSIX_LIB_STRUCT_NAME.gates[i];++i)
	{
		int (*libop)(enum library_operation op, const char** error, int max_length);
		libop = POSIX_LIB_STRUCT_NAME.gates[i]->library_operation;
		if(NULL != libop)
			libop(library_before_load, error, max_length);
	}

	return 0;
}

#ifdef LXC_EMBED_MODULE_POSIX
	struct lxc_loadable_library logxcontroll_loadable_library_posix
#else
	struct lxc_loadable_library logxcontroll_loadable_library
#endif
=
{
	.library_operation = posix_libop,
	.signals = (Signal[])
	{
		&lxc_posix_sockaddr,

		NULL
	},

	//.constants =
};
