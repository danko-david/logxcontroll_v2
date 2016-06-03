/*
 * platform.c
 *
 *  Created on: 2016.05.26.
 *      Author: szupervigyor
 */

#include "core/logxcontroll.h"

void lxc_load_embedded_modules
(
	const char** errors,
	int maxlength
)
{
	//TODO

#ifdef LXC_EMBED_MODULE_ARITHMETIC
	lxc_load_library(&logxcontroll_loadable_library_arithmetic, errors, sizeof(errors));
#endif

#ifdef LXC_EMBED_MODULE_POSIX
	lxc_load_library(&logxcontroll_loadable_library_posix, errors, sizeof(errors));
#endif

}


