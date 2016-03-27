/*
 * builtin_gate_switch.c
 *
 *  Created on: 2016.02.18.
 *      Author: szupervigyor
 */

#include "core/builtins/builtin_gate_switch.h"

typedef struct case_frame* CaseFrame;

struct case_frame
{
	LxcValue case_value;

	struct lxc_gate_switch* owner;
	struct lxc_instance output_wire_data_collector;

	IOCircuit circuit;
};

struct wire_port
{
	Signal signal;
	uint index;
	char* name;
	Wire wire;
};

struct lxc_gate_switch
{
	struct lxc_instance base;

	Signal switch_type;
	Wire switch_input;
	uint switch_input_index;

	uint input_ports_length;
	struct wire_port** input_ports;

	uint output_ports_length;
	struct wire_port** output_ports;

	uint cases_count;
	CaseFrame* cases;

	CaseFrame current_case;
};

const char* switch_gate_name()
{
	return "switch";
}

Gate create_switch_gate(const struct lxc_gate_behavior* behavior)
{
	Gate ret = malloc(sizeof(struct lxc_gate_switch));
	memset(ret, 0, sizeof(struct lxc_gate_behavior));
	lxc_init_instance(ret, behavior);
	return ret;
}

void switch_gate_destroy(/*Gate instance*/)
{
	//TODO complex task
}

int switch_get_input_types(Gate instance, Signal* arr, uint max_length)
{
	struct lxc_gate_switch* sw = (struct lxc_gate_switch*) instance;

	Signal* set = NULL;
	unsigned int set_length = 0;

	if(NULL != sw->switch_type)
	{
		array_nt_append_element((void***)&set, &set_length, (void*)sw->switch_type);
	}

	struct wire_port** input_ports = sw->input_ports;

	if(NULL != input_ports)
	{
		for(unsigned int i=0;i < sw->input_ports_length;++i)
		{
			if(NULL == input_ports[i])
				break;

			if(array_nt_contains((void**)set, set_length, (void*)input_ports[i]->signal) < 0)
			{
				array_nt_append_element((void***)&set, &set_length, (void*) input_ports[i]->signal);
			}
		}
	}

	if(max_length >= set_length)
	{
		for(unsigned int i=0;i < set_length;++i)
			arr[i] = set[i];
	}

	free(set);
	return set_length;
};

const char* find_port_label(struct wire_port** arr, uint length, Signal signal, uint index)
{
	if(NULL == arr)
		return NULL;

	for(uint i=0;i < length && NULL != arr[i]; ++i)
		if(signal == arr[i]->signal && index == arr[i]->index)
			return arr[i]->name;

	return NULL;
}

int find_max_index(struct wire_port** arr, unsigned int length, Signal signal)
{
	if(NULL == arr)
		return 0;

	uint max = 0;

	for(uint i=0;i < length && NULL != arr[i]; ++i)
		if(signal == arr[i]->signal && max > arr[i]->index)
			max = arr[i]->index;

	return max;
}

Wire* find_wire_port_location(struct wire_port** arr, uint length, Signal signal, uint index)
{
	if(NULL == arr)
			return NULL;

	for(uint i=0;i < length && NULL != arr[i]; ++i)
		if(signal == arr[i]->signal && index == arr[i]->index)
			return &(arr[i]->wire);

	return NULL;
}


const char* switch_get_input_label(Gate instance, Signal signal, uint index)
{
	struct lxc_gate_switch* sw = (struct lxc_gate_switch*) instance;
	if(signal == sw->switch_type && index == sw->switch_input_index)
		return "switch";

	return find_port_label(sw->input_ports, sw->input_ports_length, signal, index);
}

int switch_get_input_max_index(Gate instance, Signal type)
{
	struct lxc_gate_switch* sw = (struct lxc_gate_switch*) instance;
	return find_max_index(sw->input_ports, sw->input_ports_length, type);
}

Wire switch_get_input_wire(Gate instance, Signal type, uint index)
{
	struct lxc_gate_switch* sw = (struct lxc_gate_switch*) instance;
	if(type == sw->switch_type && index == sw->switch_input_index)
		return sw->switch_input;

	Wire* ret = find_wire_port_location(sw->input_ports, sw->input_ports_length, type, index);

	return NULL == ret? NULL: *ret;
}

int switch_wire_input(Gate instance, Signal signal, Wire wire, uint index)
{
	struct lxc_gate_switch* sw = (struct lxc_gate_switch*) instance;

	if(signal == sw->switch_type && index == sw->switch_input_index)
	{
		sw->switch_input = wire;
		return 0;
	}

	Wire* find = find_wire_port_location(sw->input_ports, sw->input_ports_length, signal, index);

	if(NULL != find)
	{
		*find = wire;
		return 0;
	}

	return LXC_ERROR_PORT_OUT_OF_RANGE;
}

void switch_change_case(struct lxc_gate_switch* sw, CaseFrame case_frame)
{
	sw->current_case = case_frame;

	if(NULL == case_frame)
		return;

	unsigned int len = sw->input_ports_length;
	struct wire_port** in = sw->input_ports;

	if(len != case_frame->circuit->inputs_count)
	{
		printf("ERROR: switch input size does't fit with the case input size.\r\n");
	}

	Wire* f_wire = case_frame->circuit->inputs;

	for(uint i=0;i<len;++i)
	{
		//end of wires
		if(NULL == in[i])
			break;

		//no source wire set
		if(NULL == in[i]->wire)
			continue;

		//end of target wires
		if(NULL == f_wire[i])
			return;

		if(in[i]->signal != f_wire[i]->type)
		{
			printf("Case target wire[%d] type is different!\r\n",i);
			return;
		}

		lxc_drive_wire_value(NULL, -1, f_wire[i], in[i]->wire->current_value);
	}
}

void switch_input_value_changed(Gate instance, Signal type, LxcValue value, unsigned int index)
{
	struct lxc_gate_switch* sw = (struct lxc_gate_switch*) instance;

	if(NULL == type)//on enable
		return;

	//modify selected case
	if(type == sw->switch_type && index == sw->switch_input_index)
	{
		if(NULL == value || NULL == value->type || NULL == value->type->equals)
			return;

		bool (*eq)(LxcValue, LxcValue) = value->type->equals;

		unsigned int len = sw->cases_count;
		CaseFrame* cases = sw->cases;

		for(unsigned int i = 0;i < len;++i)
		{
			if(NULL == cases[i])
				return;

			if(eq(cases[i]->case_value, value))
			{
				switch_change_case(sw, cases[i]);
				return;
			}
		}
	}

	CaseFrame current = sw->current_case;

	if(NULL == current)
		return;

	//modify input value
	Wire* input = find_mixed_wire_location(current->circuit->inputs, current->circuit->inputs_count, type, index);

	if(NULL == input)
		return;

	lxc_drive_wire_value(instance, -1, *input, value);
}

int switch_get_output_types(Gate instance, Signal* arr, unsigned int max_length)
{
	struct lxc_gate_switch* sw = (struct lxc_gate_switch*) instance;

	Signal* set = NULL;
	uint set_length = 0;

	struct wire_port** output_ports = sw->output_ports;

	if(NULL != output_ports)
	{
		for(uint i=0;i < sw->output_ports_length;++i)
		{
			if(NULL == output_ports[i])
				break;

			if(array_fix_contains((void**)set, set_length, (void*)output_ports[i]->signal) < 0)
			{
				array_nt_append_element((void***)&set, &set_length, (void*)output_ports[i]->signal);
			}
		}
	}

	if(max_length >= set_length)
	{
		for(uint i=0;i < set_length;++i)
			arr[i] = set[i];
	}

	free(set);
	return set_length;
};

const char* switch_get_output_label(Gate instance, Signal signal, unsigned int index)
{
	struct lxc_gate_switch* sw = (struct lxc_gate_switch*) instance;
	return find_port_label(sw->output_ports, sw->output_ports_length, signal, index);
}

int switch_get_output_max_index(Gate instance, Signal type)
{
	struct lxc_gate_switch* sw = (struct lxc_gate_switch*) instance;
	return find_max_index(sw->output_ports, sw->output_ports_length, type);
}

Wire switch_get_output_wire(Gate instance, Signal type, unsigned int index)
{
	struct lxc_gate_switch* sw = (struct lxc_gate_switch*) instance;
	Wire* ret = find_wire_port_location(sw->output_ports, sw->output_ports_length, type, index);
	return NULL == ret? NULL: *ret;
}

int switch_wire_output(Gate instance, Signal signal, Wire wire, unsigned int index)
{
	struct lxc_gate_switch* sw = (struct lxc_gate_switch*) instance;

	Wire* find = find_wire_port_location(sw->output_ports, sw->output_ports_length, signal, index);


	if(NULL != find)
	{
		*find = wire;
		return 0;
	}

	return LXC_ERROR_PORT_OUT_OF_RANGE;
}

bool is_port_exists(char* name, struct wire_port** port, uint length)
{
	if(NULL == port)
		return false;

	for(uint i=0;i< length;++i)
	{
		if(NULL == port[i])
			return false;

		if(strcmp(port[i]->name, name) == 0)
			return true;
	}

	return false;
}

int max_index_by_type(struct wire_port** port, uint length, Signal type)
{
	if(NULL == port)
		return -1;

	int max = -1;

	for(uint i=0;i < length;++i)
	{
		if(NULL == port[i])
			return max;

		if(port[i]->signal == type && max < (int) port[i]->index)
			max = port[i]->index;
	}

	return max;
}

bool check_name_validity(char* name, bool may_switch)
{
	return NULL != name && strlen(name) != 0 && (may_switch || strcmp(name, "switch") != 0);
}

//add port to the input and to all iocircuit
int switch_add_port(bool in, struct lxc_gate_switch* sw, char* name, Signal type)
{
	if(!check_name_validity(name, false))
		return LXC_ERROR_ILLEGAL_NAME;

	if(is_port_exists(name, sw->input_ports, sw->input_ports_length))
		return LXC_ERROR_INPUT_PORT_ALREADY_EXISTS;

	if(is_port_exists(name, sw->output_ports, sw->output_ports_length))
		return LXC_ERROR_OUTPUT_PORT_ALREADY_EXISTS;

	struct wire_port* port = malloc(sizeof(struct wire_port));

	struct wire_port*** subject = in? &(sw->input_ports):&(sw->output_ports);
	uint* subject_length = in?&(sw->input_ports_length):&(sw->output_ports_length);

	port->index = max_index_by_type(*subject, *subject_length, type) + 1;
	port->name = copy_string(name);
	port->signal = type;
	array_fix_add_element((void***)subject, subject_length, (void*) port);

	return 0;
}

int find_port_index_by_name(struct wire_port** ports, uint length, char* name)
{
	if(NULL == ports)
		return -1;

	for(uint i=0;i<length;++i)
		if(strcmp(name, ports[i]->name) == 0)
			return i;

	return -1;
}

int switch_modify_switch_input(struct lxc_gate_switch* sw, struct switchctl* ctl)
{

}

int switch_modify_port(struct lxc_gate_switch* sw, struct switchctl* ctl)
{
	if(!check_name_validity(ctl->name, true))
		return LXC_ERROR_ILLEGAL_NAME;

	if(strcmp("switch", ctl->name))
		return switch_modify_switch_input(sw, ctl);

	bool dir;
	int index = -1;

	{
		int i = find_port_index_by_name(sw->input_ports, sw->input_ports_length, ctl->name);
		if(i >= 0)
		{
			dir = DIRECTION_IN;
			index = i;
		}
	}

	if(-1 == index)
	{
		int i = find_port_index_by_name(sw->output_ports, sw->output_ports_length, ctl->name);
		if(i >= 0)
		{
			dir = DIRECTION_OUT;
			index = i;
		}
	}

	if(-1 == index)
		return LXC_ERROR_PORT_DOESNOT_EXISTS;

	switch(ctl->kind)
	{
	case SWITCH_CTL_KIND_NAME:
	case SWITCH_CTL_KIND_TYPE:
	case SWITCH_CTL_KIND_VALUE:
	case SWITCH_CTL_KIND_DIRECTION:
	case SWITCH_CTL_KIND_CIRCUIT:

	default:
		return LXC_ERROR_ILLEGAL_REQUEST;
	}

}

int switch_gatectl(Gate instance, unsigned long request, struct switchctl* ctl)
{
	switch(request)
	{
	case SWITCH_CTL_ADD_INPUT://name, type
		return switch_add_port(DIRECTION_IN, (struct lxc_gate_switch*) instance, ctl->name, ctl->type);

	case SWITCH_CTL_ADD_OUTPUT://name, type
		return switch_add_port(DIRECTION_OUT, (struct lxc_gate_switch*) instance, ctl->name, ctl->type);

	case SWITCH_CTL_MODIFY_PORT://name, kind, union{name, type, direction}

		break;

	case SWITCH_CTL_REMOVE_PORT://name

		break;

	case SWITCH_CTL_CREATE_CASE://name, value,

		break;

	case SWITCH_CTL_SET_OF_CASE://name, kind, union{name, value}

		break;

	case SWITCH_CTL_GET_OF_CASE://name, kind, union{name, value, iocircuit}

		break;

	case SWITCH_CTL_REMOVE_CASE://name

		break;
	};

	return LXC_ERROR_GATECTL_BAD_CALL;
}


const struct lxc_gate_behavior builtin_gate_switch =
{
	.gate_name = "switch",
	.create = create_switch_gate,
	.destroy = switch_gate_destroy,
	.get_input_types = switch_get_input_types,
	.get_input_label = switch_get_input_label,
	.get_input_max_index = switch_get_input_max_index,
	.get_input_wire = switch_get_input_wire,
	.wire_input = switch_wire_input,
	.input_value_changed = switch_input_value_changed,
	.execute = NULL,
	.get_output_types = switch_get_output_types,
	.get_output_label = switch_get_output_label,
	.get_output_max_index = switch_get_output_max_index,
	.get_output_wire = switch_get_output_wire,
	.wire_output = switch_wire_output,
//	.enumerate_properties = switch_enumerate_properties,
/*
		const char* (*get_property_label)(Gate instance, const char* property);
		const char* (*get_property_description)(Gate instance, const char* property);

	.get_property_value = switch_get_property_value,
*/
	.gatectl = switch_gatectl,
	//int (*set_property)(Gate instance, const char* property, char* value, char* err, int max_length);
};


