
#include "core/logxcontroll.h"

//pure functional can be a lay is the underlying function operates on environment
struct lxc_pure_functional_behavior
{
	struct lxc_generic_portb_behavior base;
	void (*function)(void**, LxcValue*);
};

struct lxc_import_tmp_storage
{
	//TODO LxcValueContainer
	Tokenport port;
	LxcValue value;
};


struct lxc_port_definition
{
	Signal type;
	int subtype;
	const char* name;
};

struct lxc_simple_func_gate_builder
{
	char* name;
	//the parameters is an array of pointers of the specified input
	//LxcValue's value and a series of LxcValue* (storage for output value)
	//function must be synchronous
	void (*function)(void**, LxcValue*);
	struct lxc_port_definition** inputs;
	struct lxc_port_definition** outputs;
};

struct lxc_pure_functional_behavior* lxc_generic_create_simple_function_wrapper
(
	struct lxc_simple_func_gate_builder* builder
);

struct lxc_pure_functional_behavior* lxc_generic_shorthand_func_wrap
(
	char* name,
	void (*funct)(void** param, LxcValue* val),
	...
);


