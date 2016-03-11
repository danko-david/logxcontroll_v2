/*
 * libao.c
 *
 *  Created on: 2016.02.24.
 *      Author: szupervigyor
 */

#include "libao.h"

static Signal signal_pcm;


struct lxc_generic_ib_behavior* audio_playback;

/*
struct detailed_gate_entry* audio_playback_gate =
{
	.behavior = &audio_playback,
	.generic_name = "audio playback",
	.paths = (char**[])
	{
		(char*[]){"device", "sound", NULL},

		NULL,
	}


	//TODO
}

int lib_op(enum library_operation op, char*** errors)
{
	signal_pcm = lxc_get_signal_by_name("pcm");
	if(NULL == signal_pcm)
	{
		array_pnt_append_element((void***)errors, copy_string("Signal `pcm` doesn't exists."));
		return 1;
	}



	audio_playback = malloc(sizeof(struct lxc_generic_ib_behavior));
	memcpy(audio_playback, &lxc_generic_ib_prototype, sizeof(struct lxc_generic_ib_behavior));

	audio_playback->gate_name = "audio playback";
	audio_playback->base.execute = lxc_libao_playback_execute,


	audio_playback->instance_memory_size = sizeof(struct lxc_libao_playback);
	audio_playback->instance_destroy = lxc_libao_destory_playback_gate;


	ao_initialize();


	return 0;
}

struct loadable_library logixcontroll_loadable_library_libao =
{
	.library_operation = lib_op,
	.detailed_gate_entry = (struct detailed_gate_entry*[])
	{
		&audio_playback,

		NULL,
	}




};


void lxc_libao_init_playback_gate(Gate instance)
{
	struct lxc_libao_playback* in = (struct lxc_libao_playback*) instance;

	//TODO később is lehet állítani.
	int default_driver = ao_default_driver_id();
	memset(&(in->format), 0, sizeof(in->format));

	in->format.bits = 16;
	in->format.channels = 1;
	in->format.rate = 22050;
	in->format.byte_format = AO_FMT_LITTLE;

	in->device = ao_open_live(default_driver, &(in->format), NULL );

	if (NULL == in->device)
	{
		in->error = errno;
	}
}

Gate lxc_libao_create_playback_gate()
{
	return malloc_zero(sizeof(struct lxc_libao_playback));
}

void lxc_libao_destory_playback_gate(struct lxc_generic_ib_instance* instance)
{
	UNUSED(instance);
	//TODO
}

void lxc_libao_playback_execute(Gate instance, Signal type, LxcValue value, uint index)
{

}

/*

 struct soundOut* startSound()
{
	struct soundOut* ret = malloc(sizeof(struct soundOut));
	int default_driver;

	ao_initialize();
	default_driver = ao_default_driver_id();
	memset(&(ret->format), 0, sizeof(ret->format));

	ret->format.bits = 16;
	ret->format.channels = 1;
	ret->format.rate = 22050;
	ret->format.byte_format = AO_FMT_LITTLE;

	ret->device = ao_open_live(default_driver, &(ret->format), NULL );

	if (NULL  == ret->device)
	{
		free(ret);
		return NULL;
	}

	return ret;
}

void playSound(struct soundOut* out, void* data, size_t len)
{
	ao_play(out->device, data, len);
}

void cleanupSound(struct soundOut* out)
{
	ao_close(out->device);
	free(out);
	ao_shutdown();
}

 */
