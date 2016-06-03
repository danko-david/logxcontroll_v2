/*
 * liblxc_posix.h
 *
 *  Created on: 2016.03.05.
 *      Author: szupervigyor
 */

#ifndef LIBLXC_POSIX_H_
#define LIBLXC_POSIX_H_

#include "core/logxcontroll.h"

#include <sys/types.h>
#include <sys/socket.h>


#ifdef LXC_EMBED_MODULE_POSIX
	#define POSIX_LIB_STRUCT_NAME logxcontroll_loadable_library_posix
	extern struct lxc_loadable_library logxcontroll_loadable_library_posix;
#else
	#define POSIX_LIB_STRUCT_NAME logxcontroll_loadable_library
	extern struct lxc_loadable_library logxcontroll_loadable_library;
#endif

extern const struct lxc_signal_type lxc_posix_sockaddr;

/************************ Detailed gate entries ********************************/

extern const char*** lxc_posix_path_socket;

//sockaddr_in, sockaddr_un,
extern void* produce_posix_socket_create();

extern void* produce_posix_socket_bring_up();

extern void* produce_posix_socketaddress_create();

//extern void* produce_posix_socket_accept();

//extern void* produce_posix_serial();

//extern void* produce_posix_fd_io();

//extern void* produce_posix_cast_to_iostream();



#endif /* LIBLXC_POSIX_H_ */
