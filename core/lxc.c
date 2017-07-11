

//#include "lxc.h"

#include "core/logxcontroll.h"

//int lxc_register_gate_type(struct lxc_gate_behavior* behavior);

void lxc_init_instance(Gate instance, const struct lxc_gate_behavior* behavior)
{
	instance->behavior = behavior;
	instance->enabled = 0;
	instance->execution_behavior = lxc_execution_default_behavior;
}

void lxc_execution_default_behavior(Gate instance, Signal type, int subtype, LxcValue value, uint index)
{
	instance->behavior->execute(instance, type, subtype, value, index);
}

void lxc_gate_destroy_simple_free(Gate instance)
{
	free(instance);
}
