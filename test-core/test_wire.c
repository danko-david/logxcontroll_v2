

#include "test-core/test_core.h"

Wire lxc_test_create_wire(Signal sig)
{
	Wire w = lxc_create_wire(sig);
	NP_ASSERT_NOT_NULL(w);
	return w;
}

void lxc_test_destroy_wire(Wire w)
{
	//TODO test wire busy (has wired gates tokenports in use), releaslues, etc.

	if(NULL != w->ref_des)
	{
		free(w->ref_des);
	}

	if(NULL != w->drivens)
	{
		free(w->drivens);
	}

	if(NULL != w->drivers)
	{
		free(w->drivers);
	}

	free(w);
}

/**
 * Wire functionality:
 *
 *
 *
 *
 *
 * */



/**
 * Testing with novaprova/valdrind, this test assures functionality and
 *	and test for memory leakage.
 * */
static void test_wire_create_destory(void)
{
	Wire vint = lxc_test_create_wire(&lxc_signal_int);
	lxc_test_destroy_wire(vint);
}

//TODO tokenPort


//
//TODO available
//TODO acquire_token Tokenport lxc_get_value_from_tokenport_array
//TODO lxc_absorb_token
//TODO release_token

//TODO test event: driving value, (un)wiring, modify attributes


//TODO test blocking: overproducing test.
//TODO tokenport: multiport consumption

