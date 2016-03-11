

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

Wire lxc_create_wire(Signal type)
{
	Wire ret = malloc(sizeof(struct lxc_wire));
	ret->type = type;
	ret->current_value = NULL;

	ret->drivers = NULL;
	ret->drivers_length = 0;

	ret->drivens = NULL;
	ret->drivens_length = 0;

	return ret;
}

int private_generic_wiring
(
	Signal signal,
	Wire wire,
	Gate instance,
	int index,
	int (*get_max_index)(Gate instance, Signal type),
	int (*get_types)(Gate instance, Signal* arr, int max_length),
	Wire (*get_wire)(Gate instance, Signal type, int index),
	int (*wire_function)(Gate instance, Signal signal, Wire wire, int index),
	void (*on_success)(Signal type, Wire wire, Gate g, int index)
)
{

	//is the call legal?
	if(NULL == signal && NULL == wire)
		return LXC_ERROR_BAD_CALL;

	//setting value if missing
	if(NULL == signal)
	{
		signal = wire->type;
	}
	//signal is not null. But if the wire not null it is use the same type?
	else if(NULL != wire && signal != wire->type)
	{
		return LXC_ERROR_BAD_CALL;
	}

	//Check signal is supported
	Signal supp[LXC_GATE_MAX_IO_TYPE_COUNT];

	int supp_max = get_types(instance, supp, LXC_GATE_MAX_IO_TYPE_COUNT);

	if(supp_max < -LXC_GATE_MAX_IO_TYPE_COUNT)
		return LXC_ERROR_TOO_MANY_TYPES;

	int i = 0;

	while(i < supp_max)
	{
		if(supp[i] == signal)
			goto passed;

		++i;
	}

	return LXC_ERROR_TYPE_NOT_SUPPORTED;

passed:


	if(index < 0 || index > get_max_index(instance, signal))
	{
		return LXC_ERROR_PORT_OUT_OF_RANGE;
	}

	Wire wired = get_wire(instance, signal, index);

	//now user tries to unwire with specified index
	if(NULL == wire)
	{
		if(NULL == wired)
		{
			return LXC_ERROR_PORT_IS_ALREADY_FREE;
		}

		wire_function(instance, signal, NULL, index);
	}
	else
	{
		if(NULL != wired)
		{
			return LXC_ERROR_PORT_IS_IN_USE;
		}

		wire_function(instance, signal, wire, index);
	}

	//int ret = private_check_signal(g, &type, wire, g->behavior->get_input_types);

	//mi van ha a vezetéken rajta van de ő azt mondja hogy nincs?
	//vagy ha már a vezetéken rajta van de ő azt mondja nem?

	//unwire gate
	if(NULL == wire)
	{
		//unwire the input.
		bool result = remove_port(instance, index, &(wire->drivens), &(wire->drivens_length));
		if(!result)
		{
			return LXC_ERROR_NOT_CONNECTED_UNWIRING;
		}
	}

	on_success(signal, wire, instance, index);
	return 0;
}

//TODO synchronize
int remove_port(Gate instance, uint index, Port** ports, int* length)
{
	if(NULL == *ports)
	{
		return false;
	}
	else
	{
		int len = *length;
		int i = 0;
		while(i < len)
		{
			Port p = (*ports)[i];
			if(instance == p->gate && index == p->index)
			{
				//i found them
				free(p);

				//shifting the array if least one element on the upper index
				--len;
				//len is now lower, no extra len+1 needed in the next ops.
				if(i < len)
				{
					while(i < len)
					{
						(*ports)[i] = (*ports)[i+1];
						++i;
					}
				}

				//set the last element NULL
				//(no duplication at the end of the array)
				(*ports)[len] = NULL;
				return 1;
			}

			++i;
		}


		return 0;
	}
}

//TODO synchronize
void add_port(Port add, Port** ports, uint* length)
{
	array_nt_append_element((void***)ports, length, add);
}

void on_successfull_input_wiring(Signal type, Wire wire, Gate g, int index)
{
	//if successfully wired/unwired, register/deregister into the wire.
	//and notify the wire writer gate to re execute itself
	if(NULL != wire)
	{
		Port p = malloc(sizeof(struct lxc_port));
		p->gate = g;
		p->index = index;
		add_port(p, &(wire->drivens), &(wire->drivens_length));

		//TODO how to notify in this case? the driver or the driven gate?
		//notify_drivers(wire);

		//TODO in this case reference count management
		if(g->enabled)
			g->behavior->input_value_changed(g, type, wire->current_value, index);
	}
	else
	{
		//this case is clear, notify unwired input
		g->behavior->input_value_changed(g, type, NULL, index);
	}
}

void on_successfull_output_wiring(Signal type, Wire wire, Gate g, int index)
{
	if(NULL != wire)
	{
		//new wire attached, send to the driver a general input_change
		Port p = malloc(sizeof(struct lxc_port));
		p->gate = g;
		p->index = index;
		add_port(p, &(wire->drivers), &(wire->drivers_length));

		if(g->enabled)
			g->behavior->input_value_changed(g, NULL, NULL, 0);
	}
	else
	{
		//notify driven gates, input is unwired
		Port* ps = wire->drivens;
		int len = wire->drivens_length;
		int i = 0;
		while(i<len)
		{
			Port p = ps[i];
			if(NULL == p)
				return;

			Gate target = p->gate;
			if(target->enabled)
				target->behavior->input_value_changed(target, type, NULL, p->index);
		}
	}
}

int lxc_wire_gate_input(Signal type, Wire wire, Gate g, uint index)
{
	//at this point type is maybe modified, but to a legal value.
	return private_generic_wiring
	(
		type,
		wire,
		g,
		index,
		g->behavior->get_input_max_index,
		g->behavior->get_input_types,
		g->behavior->get_input_wire,
		g->behavior->wire_input,
		on_successfull_input_wiring
	);
}

int lxc_wire_gate_output(Signal type, Wire wire, Gate g, uint index)
{
	return private_generic_wiring
	(
		type,
		wire,
		g,
		index,
		g->behavior->get_output_max_index,
		g->behavior->get_output_types,
		g->behavior->get_output_wire,
		g->behavior->wire_output,
		on_successfull_output_wiring
	);
}


void lxc_drive_wire_value(Gate instance, uint out_index, Wire wire, LxcValue value)
{
	//TODO framework debugging mode, update wire value on change

	//tryfree old value

	int i = 0;
	Port* ports = wire->drivens;
	Signal signal = wire->type;
	int len = wire->drivens_length;

	//TODO reference value
	wire->current_value = value;
	while(i < len)
	{
		Port p = ports[i];
		if(NULL == p)
			return;

		if(p->gate->enabled)
			p->gate->behavior->input_value_changed(p->gate, signal, value, p->index);

		++i;
	}
	//TODO unreferece value and free if no more ref.
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

LxcValue lxc_get_wire_value(Wire w)
{
	return w->current_value;
}

void* lxc_get_value(LxcValue v)
{
	if(NULL == v)
		return NULL;

	return v->operations->data_address(v);
}
