

//#include "lxc.h"

#include "core/logxcontroll.h"

//int lxc_register_gate_type(struct lxc_gate_behavior* behavior);

void lxc_init_instance(Gate instance, const struct lxc_gate_behavior* behavior)
{
	instance->behavior = behavior;
	instance->enabled = 0;
	instance->execution_behavior = default_execution_behavior;
}

void default_execution_behavior(Gate instance, Signal type, int subtype, LxcValue value, uint index)
{
	instance->behavior->execute(instance, type, subtype, value, index);
}

void lxc_destroy_simple_free(Gate instance)
{
	free(instance);
}

IOCircuit lxc_create_circuit()
{
	IOCircuit ret = malloc(sizeof(struct circuit));
	memset(ret, 0, sizeof(struct circuit));
	return ret;
}

int lxc_add_gate_to_circuit(IOCircuit circuit, Gate gate)
{
	if(NULL != circuit && NULL != gate)
	{
		array_pnt_append_element
		(
			(void***) &(circuit->gates),
			(void*)gate
		);
		return 0;
	}
	return 1;
}

void lxc_destroy_circuit(IOCircuit circuit)
{
	free(circuit);
}

Circuit lxc_create_iocircuit()
{
	Circuit ret = malloc(sizeof(struct circuit));
	memset(ret, 0, sizeof(struct circuit));
	return ret;
}

