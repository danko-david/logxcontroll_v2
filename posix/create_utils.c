/*
 * create_utils.c
 *
 *  Created on: 2016.04.02.
 *      Author: szupervigyor
 */

#ifdef INCLUDE_CREATE_UTILS_ONCE

#include <netdb.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>


/*
	TODO
		else if(AF_INET6 == family)
		{
			safe_strcpy(ret, max_length, "INET6");
			return 0;
		}
		else if(AF_UNIX == family)
		{
			safe_strcpy(ret, max_length, "UNIX");
			return 0;
		}
		else if(AF_PACKET == family)
		{
			safe_strcpy(ret, max_length, "PACKET");
			return 0;
		}
*/

static struct create_utils* new_create_utils
(
	int family,
	const char* name,
	void (*on_creator_remove)(struct lxc_posix_socketaddress_create*),
	void (*on_creator_use)(struct lxc_posix_socketaddress_create*),
	LxcValue (*create_socket)(struct lxc_posix_socketaddress_create*)
)
{
	struct create_utils* cre =
		(struct create_utils*) malloc(sizeof(struct create_utils));

	cre->family = family;
	cre->name = name;
	cre->on_creator_remove = on_creator_remove;
	cre->on_creator_use = on_creator_use;
	cre->create_socket = create_socket;

	return cre;
}

static void do_noop(struct lxc_posix_socketaddress_create* gate){}

static LxcValue create_nothing
(
	struct lxc_posix_socketaddress_create* gate
)
{
	return NULL;
}


static void inet_on_use(struct lxc_posix_socketaddress_create* gate)
{
	lxc_port_unchecked_add_new_port
	(
		&(gate->base.input_ports),
		"address",
		&lxc_signal_string
	);

	lxc_port_unchecked_add_new_port
	(
		&(gate->base.input_ports),
		"port",
		&lxc_signal_int
	);
}
static LxcValue inet_create_socket
(
	struct lxc_posix_socketaddress_create* gate
)
{
	//get all inputs
	LxcValue addr = lxc_get_value_safe_from_tokenport_array
	(
		gate->base.inputs,
		gate->base.inputs_length,
		0
	);

	if(NULL == addr)
	{
		return NULL;
	}

	LxcValue port = lxc_get_value_safe_from_tokenport_array
	(
		gate->base.inputs,
		gate->base.inputs_length,
		1
	);

	if(NULL == port)
	{
		return NULL;
	}

	struct hostent* host = gethostbyname((const char*) lxc_get_value(addr));

	if(NULL == host)
	{
		return NULL;
	}

	int port_num = *((int*)lxc_get_value(port));

	if(port_num < 0 || port_num > 65536)
	{
		return NULL;
	}

	LxcValue ret =	lxc_create_generic_value
					(
						&lxc_posix_sockaddr,
						sizeof(struct sockaddr_in)
					);

	struct sockaddr_in* setup = (struct sockaddr_in*) lxc_get_value(ret);
	setup->sin_family = AF_INET;

	bcopy
	(
		(char *) (host->h_addr_list[0]),
		(char *)&(setup->sin_addr.s_addr),
		host->h_length
	);

	//setup->sin_addr = *((struct in_addr*) host->h_addr_list[0]);
	setup->sin_port = htons(port_num);

	return ret;
}


static void register_create_utils()
{
	struct create_utils*** target = &IMPLEMENTED_ADDRESS_CREATORS;

	array_pnt_append_element
	(
		(void ***) target,
		(void*) new_create_utils
			(
				AF_UNSPEC,
				"UNSPEC",
				do_noop,
				do_noop,
				create_nothing
			)
	);

	array_pnt_append_element
	(
		(void***) target,
		(void*) new_create_utils
		(
			AF_INET,
			"INET",
			do_noop,
			inet_on_use,
			inet_create_socket
		)
	);
}

#endif
