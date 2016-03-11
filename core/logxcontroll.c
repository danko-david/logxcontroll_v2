/*
 * logxcontroll.c
 *
 *  Created on: 2016.02.27.
 *      Author: szupervigyor
 */


#include "core/logxcontroll.h"

void logxcontroll_init_environment()
{
	/*** Register built in Libraries ***/
	char errors[200];
	lxc_load_library(&logxcontroll_loadable_library_builtin, errors, 200);
	lxc_load_library(&logxcontroll_loadable_library_bool, errors, 200);

	lxc_load_shared_library("/home/szupervigyor/projektek/LogxKontroll/WS/Liblxc_ieee1003/Default/libLiblxc_ieee1003", errors, 200);

}
