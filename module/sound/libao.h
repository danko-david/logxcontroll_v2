/*
 * libao.h
 *
 *  Created on: 2016.02.24.
 *      Author: szupervigyor
 */

#ifndef LIBAO_H_
#define LIBAO_H_

#include "core/logxcontroll.h"
#include "ao/ao.h"

extern struct lxc_generic_ib_behavior* audio_playback;

void lxc_libao_init_playback_gate(Gate);

void lxc_libao_playback_execute(Gate, Signal, LxcValue, uint);

//void lxc_libao_destory_playback_gate(struct lxc_generic_ib_instance*);

const char* lxc_libao_get_playback_name();

int lib_op(enum library_operation, char*** errors);

struct lxc_libao_playback
{
	struct lxc_generic_ib_instance base;
	ao_device *device;
	ao_sample_format format;
	int error;
};

#endif /* LIBAO_H_ */
