

//#include "lxc.h"

#include "core/logxcontroll.h"

//int lxc_register_gate_type(struct lxc_gate_behavior* behavior);

void lxc_init_instance(Gate instance, const struct lxc_gate_behavior* behavior)
{
	instance->behavior = behavior;
	instance->enabled = 0;
	instance->execution_behavior = default_execution_behavior;
}

void default_execution_behavior(Gate instance, Signal type, LxcValue value, uint index)
{
	instance->behavior->execute(instance, type, value, index);
}

void lxc_destroy_simple_free(Gate instance)
{
	free(instance);
}

Circuit lxc_create_circuit()
{
	Circuit ret = malloc(sizeof(struct circuit));
	memset(ret, 0, sizeof(struct circuit));
	return ret;
}

int lxc_add_gate_to_circuit(Circuit circuit, Gate gate)
{
	if(NULL != circuit && NULL != gate)
	{
		array_nt_append_element(&(circuit->gates), &(circuit->gates_count), gate);
		return 0;
	}
	return 1;
}

void lxc_destroy_circuit(Circuit circuit)
{
	free(circuit);
}

IOCircuit lxc_create_iocircuit()
{
	IOCircuit ret = malloc(sizeof(struct iocircuit));
	memset(ret, 0, sizeof(struct iocircuit));
	return ret;
}

