
#include "core/logxcontroll.h"

static void functional_gate_execute
(
	Gate instance,
	Signal type,
	int subtype,
	LxcValue value,
	uint index
)
{
	struct lxc_pure_functional_behavior* b =
		(struct lxc_pure_functional_behavior*) instance->behavior;

	struct lxc_generic_portb_instance* g =
		(struct lxc_generic_portb_instance*) instance;

	//read all value, execute if there's no null value.

	int ins = b->base.input_ports.max_size;
	struct lxc_import_tmp_storage in[ins];

	{
		bool complete = true;
		int i = -1;
		while(++i < ins)
		{
			in[i].port = g->inputs[i];
			if(NULL != in[i].port)
			{
				in[i].value = lxc_get_token_value(in[i].port);
				if(NULL != in[i].value)
				{
					continue;
				}
			}

			complete = false;
			break;
		}

		if(!complete)
		{
			//not all value preset, wipe and return
			int erase = -1;
			while(++erase < i)
			{
				lxc_wire_release_token(in[erase].port);
			}

			return;
		}
		else
		{
			int accept = -1;
			while(++accept < ins)
			{
				lxc_absorb_token(in[accept].port);
			}
		}
	}

	//all input preset

	//TODO LxcValueContainer

	int outs = b->base.output_ports.max_size;
	LxcValue out[outs];
	memset(out, 0, sizeof(out));

	void* param[ins];
	{
		int i=-1;
		while(++i < ins)
		{
			param[i] = lxc_get_value(in[i].value);
		}
	}

	//get the numbers of output and pass a pointer array

	b->function(param, out);

	{
		//write out
		int i=-1;
		while(++i < outs)
		{
			Wire o = g->outputs[i];
			LxcValue v = out[i];
			if(NULL != o && NULL != v)
			{
				if(o->type == v->type && o->subtype == v->subtype_info)
				{
					lxc_drive_wire_value(instance, 0, o, v);
				}
			}

			if(NULL != v)
			{
				lxc_unreference_value(v);
			}
		}
	}
}

static const char*** lib_path_functional =
(const char**[])
{
	(const char*[])
	{
		"Prototype", "Functional", NULL
	},
	NULL
};

struct lxc_pure_functional_behavior* lxc_generic_create_simple_function_wrapper
(
	struct lxc_simple_func_gate_builder* builder
)
{
	struct lxc_pure_functional_behavior* ret = malloc_zero(sizeof(struct lxc_pure_functional_behavior));
	lxc_init_from_prototype
	(
		ret,
		sizeof(struct lxc_generic_portb_behavior),

		(void*)&lxc_generic_portb_prototype,
		sizeof(lxc_generic_portb_prototype)
	);

	ret->base.base.paths = lib_path_functional;
	char** name = (char**) &ret->base.base.gate_name;
	*name = builder->name;

	ret->base.instance_memory_size = sizeof(struct lxc_generic_portb_behavior);
	ret->base.base.execute = functional_gate_execute;
	ret->function = builder->function;


	{
		int i=-1;
		struct lxc_port_definition* def;
		while(NULL != (def = builder->inputs[++i]))
		{
			lxc_port_unchecked_add_new_port
			(
				&ret->base.input_ports,
				def->name,
				def->type,
				def->subtype,
				NULL
			);
		}
	}

	{
		int i=-1;
		struct lxc_port_definition* def;
		while(NULL != (def = builder->outputs[++i]))
		{
			lxc_port_unchecked_add_new_port
			(
				&ret->base.output_ports,
				def->name,
				def->type,
				def->subtype,
				NULL
			);
		}
	}

	return ret;
}

/**
 *	"in_0" Signal_int 0
 * 	"in_1" Signal_int 0
 * 	NULL
 * 	"out_0" Signal_int 0
 * 	"out_1" Signal_int 0
 * 	"out_2" Signal_int 0
 *	NULL
 *
 * */
struct lxc_pure_functional_behavior* lxc_generic_shorthand_func_wrap
(
	char* name,
	void (*funct)(void** param, LxcValue* val),
	...
)
{
	struct lxc_simple_func_gate_builder build;
	build.name = name;
	build.function = funct;

	va_list arg;
	va_start(arg, funct);

	void** in = NULL;
	{
		array_pnt_init(&in);
		void* p;
		while(NULL != (p = va_arg(arg, void*)))
		{
			struct lxc_port_definition* def =
				alloca(sizeof(struct lxc_port_definition));

			def->name = (char*) p;
			def->type = va_arg(arg, Signal);
			def->subtype = va_arg(arg, int);
			array_pnt_append_element(&in, def);
		}
	}

	void** out = NULL;
	{
		array_pnt_init(&out);
		void* p;
		while(NULL != (p = va_arg(arg, void*)))
		{
			struct lxc_port_definition* def =
				alloca(sizeof(struct lxc_port_definition));

			def->name = (char*) p;
			def->type = va_arg(arg, Signal);
			def->subtype = va_arg(arg, int);
			array_pnt_append_element(&out, def);
		}
	}

	va_end(arg);

	build.inputs = in;
	build.outputs = out;

	struct lxc_pure_functional_behavior* ret =
		lxc_generic_create_simple_function_wrapper(&build);

	free(in);
	free(out);

	return ret;
}

