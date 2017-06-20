

#include "test-core/test_core.h"

Wire lxc_test_create_wire(Signal sig)
{
	Wire w = lxc_wire_create(sig);
	NP_ASSERT_NOT_NULL(w);
	return w;
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
	lxc_wire_destroy(vint);
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

