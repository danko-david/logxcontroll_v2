

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

static void release_port_generic
(
	Gate gate,
	int (*enumerate_types)(Gate, Signal*, int*, uint),
	int (*max_of_type)(Gate, Signal, int),
	void* (*get_wire)(Gate, Signal, int, uint),
	int (*unwire)(Signal, int, Wire, Gate, uint)
)
{
	Signal sigs[20];
	int subs[20];
	int max = enumerate_types(gate, sigs, subs, 20);
	int i = 0;
	while(i < max)
	{
		int max = max_of_type(gate, sigs[i], subs[i]);
		int m = 0;
		while(m<max)
		{
			void* in = get_wire(gate, sigs[i], subs[i],  m);
			if(NULL != in)
			{
				//printf("unwiring: gate: %p, signal: %s, subtype: %d, index: %d\n", gate, sigs[i]->name, subs[i], m);
				unwire(sigs[i], subs[i], NULL, gate,  m);
			}
			++m;
		}
		++i;
	}
}

static void release_all_port(Gate gate)
{
	release_port_generic
	(
		gate,
		gate->behavior->get_input_types,
		gate->behavior->get_input_max_index,
		(void* (*)(Gate, Signal, int, uint)) gate->behavior->get_input_wire,
		lxc_wire_gate_input
	);



	release_port_generic
	(
		gate,
		gate->behavior->get_output_types,
		gate->behavior->get_output_max_index,
		(void* (*)(Gate, Signal, int, uint)) gate->behavior->get_output_wire,
		lxc_wire_gate_output
	);
}

void lxc_wire_destroy(Wire w)
{
	//TODO test wire busy (has wired gates tokenports in use), releaslues, etc.

	lxc_import_new_value(NULL, &w->current_value);

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

void lxc_destroy_circuit(IOCircuit circ)
{
	//minimal naive implementation

	{//release gates
		if(NULL != circ->gates)
		{
			int i = 0;
			while(NULL != circ->gates[i])
			{
				release_all_port(circ->gates[i]);
				++i;
			}
		}
	}

	{
		int i = 0;
		while(NULL != circ->gates[i])
		{
			free(circ->gates[i]->ref_des);

			circ->gates[i]->behavior->destroy(circ->gates[i]);
			++i;
		}

		free(circ->gates);
	}

	{
		int i = 0;
		while(NULL != circ->wires[i])
		{
			lxc_wire_destroy(circ->wires[i]);
			++i;
		}
		free(circ->wires);
	}

	free(circ->name);
	free(circ);
}

Circuit lxc_create_iocircuit()
{
	Circuit ret = malloc(sizeof(struct circuit));
	memset(ret, 0, sizeof(struct circuit));
	return ret;
}

