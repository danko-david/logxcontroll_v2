/*
 * arithmetic.h
 *
 *  Created on: 2016.06.03.
 *      Author: szupervigyor
 */

#ifndef ARITHMETIC_H_
#define ARITHMETIC_H_

#include "core/logxcontroll.h"

#ifdef LXC_EMBED_MODULE_ARITHMETIC
	extern struct lxc_loadable_library logxcontroll_loadable_library_arithmetic;
#else
	extern struct lxc_loadable_library logxcontroll_loadable_library;
#endif

#endif /* ARITHMETIC_H_ */
