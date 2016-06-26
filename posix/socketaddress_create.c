/*
 *
 * Input ports:
 *	Vary input per type
 *
 *
 * Output ports:
 *		- socket address (sockaddr)
 *
 * Properties:
 * 		family: specifies which kind of address will be produced, assembled and
 * 		published: options is an enumeration:
 * 			INET		- IPv4 (sockaddr_in)
 * 			INET6		- IPv6 (sockaddr_in6)
 * 			UN			- UNIX Doamin Socket (sockaddr_un)
 *			DLI			- Datalink interface (sockaddr_dl)
 *			PACKET		- OSI lvl 2 link (sockadd_ll)
 *
 * 			BLUETOOTH	- Bluetooth (sockaddr_rc)
 *
 * 	TODO: CAN, PPPOX
 *
 *
 * socketaddress_create.c
 *
 *  Created on: 2016.04.02.
 *      Author: szupervigyor
 */

#include "liblxc_posix.h"

struct lxc_posix_socketaddress_create;

struct create_utils
{
	int family;
	const char* name;
	void (*on_creator_remove)(struct lxc_posix_socketaddress_create*);
	void (*on_creator_use)(struct lxc_posix_socketaddress_create*);
	LxcValue (*create_socket)(struct lxc_posix_socketaddress_create*);
};

static struct create_utils** IMPLEMENTED_ADDRESS_CREATORS;

struct lxc_posix_socketaddress_create
{
	struct lxc_generic_porti_instance base;
	int family;
	struct create_utils* current_creator;
	void* spec;
};

#define INCLUDE_CREATE_UTILS_ONCE
#include "create_utils.c"
#undef INCLUDE_CREATE_UTILS_ONCE

static int socketaddress_create_libop
(
	enum library_operation op,
	const char** errors,
	int max_length
);

static struct lxc_generic_porti_propb_behavior posix_socketaddress_create =
{
	.base.base.library_operation = socketaddress_create_libop,
};

static void socketaddress_create_execute
(
	Gate instance,
	Signal type,
	LxcValue value,
	uint index
)
{
	struct lxc_posix_socketaddress_create* gate =
		(struct lxc_posix_socketaddress_create*) instance;

	//sensitive only for output wiring event and input modify

	bool pass = false;

	if(type == &lxc_signal_system)
	{
		struct lxc_system_event* sys =
			(struct lxc_system_event*) lxc_get_value(value);

		enum lxc_system_event_type type = sys->event_type;
		if(system_event_gate_enabled == type)
		{
			pass = true;
		}
		else if(system_event_output_wire_added == type)
		{
			pass = true;
		}
	}
	else
	{
		pass = true;
	}

	if(!pass)
	{
		return;
	}


	if(NULL != gate->current_creator)
	{
		//is any output?
		//with port manager it's sure that 0'th will be the only output
		Wire w = gate->base.outputs[0];
		if(NULL == w)
		{
			return;
		}

		LxcValue addr = gate->current_creator->create_socket(gate);
		if(NULL != addr)
		{
			lxc_reference_value(addr);
			lxc_drive_wire_value(gate, 0, w, addr);
		}
	}
}

static void* socketaddress_create_access_property(Gate gate, const char* name)
{
	if(0 == strcmp(name, "family"))
	{
		return &(((struct lxc_posix_socketaddress_create*) gate)->family);
	}

	return NULL;
};

static int sockaddress_create_property_validator
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
	if(direction == DIRECTION_OUT)
	{
		int family = *((int*)addr);

		int i;
		for(i=0;NULL != IMPLEMENTED_ADDRESS_CREATORS[i];++i)
		{
			struct create_utils* cre = IMPLEMENTED_ADDRESS_CREATORS[i];
			if(family == cre->family)
			{
				safe_strcpy(ret, max_length, cre->name);
				return 0;
			}
		}

		safe_strcpy(ret, max_length, "");
		return LXC_ERROR_ENTITY_NOT_FOUND;
	}
	else
	{
		//setting propterty is available only if no input ports connected
		struct lxc_posix_socketaddress_create* gate =
				(struct lxc_posix_socketaddress_create*) instance;

		//select new creator_utils

		//unset not supported, only modifying.
		if(NULL == value)
		{
			safe_strcpy(ret, max_length, "");
			return LXC_ERROR_BAD_CALL;
		}

		struct create_utils* new_create = NULL;

		int i;
		for(i=0;NULL != IMPLEMENTED_ADDRESS_CREATORS[i];++i)
		{
			struct create_utils* cre = IMPLEMENTED_ADDRESS_CREATORS[i];
			if(0 == strcmp(cre->name, value))
			{
				new_create = cre;
				break;
			}
		}

		//requested type not found
		if(NULL == new_create)
		{
			safe_strcpy(ret, max_length, "");
			return LXC_ERROR_ENTITY_NOT_FOUND;
		}

		if(gate->family == new_create->family)
		{
			safe_strcpy(ret, max_length, "");
			return LXC_ERROR_NOTHING_CHANGED;
		}

		//call remove_hook
		if(NULL != gate->current_creator)
		{
			gate->current_creator->on_creator_remove(gate);
		}

		gate->family = new_create->family;
		gate->current_creator = new_create;

		if
		(
			lxc_port_is_any_wire_connected
			(
				&(gate->base.input_ports),
				gate->base.inputs,
				gate->base.inputs_length
			)
		)
		{
			safe_strcpy(ret, max_length, "Family can be modified only if no input port connected, disconnect them.");
			return LXC_ERROR_PORT_IS_IN_USE;
		}

		//remove all port
		lxc_port_wipe_all(&(gate->base.input_ports));

		new_create->on_creator_use(gate);

		return 0;
	}
}

static void on_new_gate(struct lxc_generic_porti_instance* gate)
{
	lxc_port_unchecked_add_new_port
	(
		&(gate->output_ports),
		"sockaddr",
		&lxc_posix_sockaddr,
		NULL
	);
}

static int socketaddress_create_libop
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
			(void*) &posix_socketaddress_create,
			sizeof(posix_socketaddress_create),

			(void*) &lxc_generic_porti_propb_prototype,
			sizeof(lxc_generic_porti_propb_prototype)
		);

		posix_socketaddress_create.base.base.library_operation =
				socketaddress_create_libop;

		posix_socketaddress_create.base.base.gate_name = "socketaddress create";

		posix_socketaddress_create.base.base.paths = lxc_posix_path_socket;

		posix_socketaddress_create.base.instance_memory_size =
					sizeof(struct lxc_posix_socketaddress_create);

		posix_socketaddress_create.base.instance_init = on_new_gate;

		posix_socketaddress_create.base.base.execute =
			socketaddress_create_execute;

		posix_socketaddress_create.properties.access_property =
				socketaddress_create_access_property;

		//this located in create_utils.c
		register_create_utils();

		lxc_add_property
		(
			&(posix_socketaddress_create.properties),
			"family",
			"Family",
			"Specifies, which kind of address will be created, like: INET for IPv4 adresses, INET6 for IPv6, UN for UNIX Doamin sockets, PACKET for low level communication, BLUETOOTH for bluetooth devices and so on.",
			"UNSPEC",
			sockaddress_create_property_validator
		);
	}
	return 0;
}

void* produce_posix_socketaddress_create()
{
	return (void*) &posix_socketaddress_create;
}
