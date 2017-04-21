/*
 * logx.h
 *
 *  Created on: 2016.01.07.
 *      Author: szupervigyor
 */

/**
 * Design decisions:
 *	speed of constructed system:
 *		wired system which is not under edit or
 *		debug must comes with full operation speed.
 *		sometimes this comes with some hacky solution like extending the
 *		base lxc_instance by declaring the "descendant" with a leading non
 *		pointer type of struct lxc_insance.
 *
 *
 *	support multithread solutions:
 *		//TODO there is some alternatives for data propagation like:
 *		self execution, it must check for stack overflow before it's occurred.
 *			a ring oscillator can break the entire runtime.
 *
 *		different execution queue for different gates (as settings)
 *
 *		simple queue which follow the well know data propagation behavior.
 *
 *		or
 *			custom execution queues like "execute once last"
 *			where input_value_changed notifies multiple time the same
 *			gate, but evaluation applied only once, last time.
 *
 *		or
 *			useful for process data parallel from a single input.
 *
 *	support IPC data types:
 *		try to linearize LxcValue data structures so you can place or copy
 *		to shared memory region or into a message.
 *
 *		I mean try to design LxcValues to use continuous memory segment, and
 *		relative pointer. (Edit: likely unachievable with real life data
 *			structure so better to take more care about serialization/cloning
 *			(with a little bit reflection) to achieve the original goal:
 *			can transfer data between processes)
 *
 *
 *
 *	Notes:
 *		You may wonder about what's whit this subtype shit?
 *		I prefer type safety.
 *		(So just raw array and struct and matrix and whateveryouwant types
 *		without notating type inside is not come into mind.)
 *		I had to decide between two solution:
 *		Register every single subtype as signal
 *			(like struct image, struct termios, char[], int[], bool[],
 *				int matrix, double matrix, whateverstorage whatevertype)
 *	OR
 *
 *		i use this subtype solution: the system has main types:
 *			- int, double, data, etc (this primitives has no meaning if have subtypes,
 *				yet i have no idea for it's porpuse)
 *			- complex types like: array, struct, (later more added like: complex, matrix,)
 *
 *		Every single main type has an ordinal (always greater than 0) which assigned at signal register time.
 *		for example: char => 1, int =>2, data=> 3
 *		So if the type is array and subtype is 2 that's mean you have an array of ints (LxcValue and int data).
 *
 *		Structures has a registering facility in accounting.{h,c} and also an ordinal assigned
 *		to every single registered struct. But this ordinals are less than 0.
 *
 *		0 ordinal (and subtype) is reserved, and means: not matter. So by convention
 *		A gate with struct input signal and 0 subtype means any subtype can be assigned.
 *		LOL smash type safety.
 *		it's a good "backdoor" for some porpuse. eg for serialization.
 *
 *		TODO: concept will be more consist if i create "any" type with hard coded
 *		0 ordinal and specify for the port manager to accept any kind of wire for
 *		this port. I have an idea about a built in serialization technic.
 *		Casting types are planned right now, another idea is to define a visitor
 *		value creator, builder methods, which provide a generic way to discover/
 *		search/iterate through any kind of type.
 *
 *		That's means we provide a generic way, and loadable modules may implement
 *		it's own data {,de}serializing method like JSON, BSON, msgpack etc.
 *
 *		At the end you need a pair of gate:
 *			- which accept `any` type of data and puts `data` on the output
 *			- and accepts `data` and produce `any` type
 *
 *		because the framework has cast_to method, `any` type can be casted to
 *		the underlying type (like instanceof in OO langs.)
 *
 * */
#include "core/logxcontroll.h"


#ifndef LOGX_H_
#define LOGX_H_


/******************************************************************************
 ************************* Basic types required by LXC ************************
 ******************************************************************************/

/****** Primitive types ******/
//
// linked list: `reference counted chain link`
//
struct chain_link_refc
{
	int refcount;
	struct chain_link_refc* next;
	void* addr;
};





/*
			Because of cross referencing inside types to each
					other, used types are defined here
*/
struct lxc_gate_behavior;
struct lxc_instance;

struct lxc_signal_type;
struct lxc_wire;
struct lxc_value;
struct lxc_port;

typedef unsigned int uint;

typedef struct lxc_instance* Gate;
typedef const struct lxc_signal_type* Signal;
typedef struct lxc_wire* Wire;
typedef struct lxc_value* LxcValue;
typedef struct lxc_tokenport* Tokenport;
typedef struct circuit* IOCircuit;
typedef struct circuit* Circuit;


/******************************************************************************
 ************************* LXC type related structure *************************
 ******************************************************************************/

//
// Represents a type caster to a specified `type`. The source type specified
// externally
//
struct lxc_cast_to
{
	Signal to;
	LxcValue (*cast_function)(LxcValue);
};

//
// Represents an LXC data type, contains the necessary datas about the signal
//  type and for the framework utilities.
//
struct lxc_signal_type
{
	/*
	 * The user friendly name of signal
	 */
	const char* name;

	/*
	 * Assigned by the framework at signal registering time
	 * used for subtype support
	 */
	int ordinal;

	/*
	 * Check signal equals to another.
	 */
	bool (*equals)(LxcValue, LxcValue);

	//TODO visit

	//TODO create new and build

	/**
	 * Map is array_pnt of key_value*
	 *
	 * A mapping used for casting value:
	 * Map<Signal, LxcValue (*cast_to)(LxcValue)>
	 */
	struct lxc_cast_to** cast_to;
};

struct lxc_tokenport
{
	Wire owner;
	int wire_index;

	struct chain_link_refc* current_value;

	Gate gate;
	uint index;
};

struct lxc_value_operation
{
	//frees the underling Value, even if it's referenced.
	//no type check applied!
	void (*free)(LxcValue value);

/*	//copies and return of the given value.
	//the new copy should be independent from the origin
	//and the new reference count is 0.
	//if this method null it's maybe a constant value.
*/

	//returns the underlying value's exact copy.
	//for you can modify an incoming value (in  the aspect of a gate)
	//you may not modify directly the incoming value, first it must be cloned.
	//if the underlying value is a constant and you clone that,
	//the new value must work like a regular (modifiable) value.
	LxcValue (*clone)(LxcValue dest);

	//returns the size of the value
	//if type is complex or array of element like,
	//the size is sum of all unique element.
	size_t (*size)(LxcValue value);

	//modify the reference counter with the underlying value
	//then returns the current refcount of value.
	int (*ref_diff)(LxcValue value, int num);

	//TODO visit and build

	//the data is always in the same structure, but can be aligned

	//never may null
	void* (*data_address)(LxcValue value);
};

struct lxc_value
{
	Signal type;

	/**
	 * Subtype info used for arrays and structs
	 * */
	int subtype_info;

	//type of the signal trouth some datatype may be same,
	//but same type of values may behaviors different.
	//some primitive values may be placed in a constant pool
	//text section calling free for this values may causes runtime breaking.

	//never may NULL
	const struct lxc_value_operation* operations;
};

#define LXC_GATE_MAX_IO_TYPE_COUNT 20


enum lxc_wire_operation_phase
{
	lxc_before_wire_driven,

	lxc_before_gate_notified,
	lxc_after_gate_notified,

	lxc_after_wire_driven,
};

struct lxc_wire_debug_hook_data
{
	int type;
	const char* id;
	/*MayNotNull*/ void (*wire_debug_hook)
	(
		struct lxc_wire_debug_hook_data* data,
		enum lxc_wire_operation_phase,
		Wire this_wire,
		Gate subject_gate,
		uint subject_port_index,
		LxcValue value,
		Signal type,
		int subtype
	);

	//glue anything what you need.
};

/**
 * Represents a wire, connect gates together.
 * A wire has driver(s) and driven gates.
 *
 * The rest of the dataflow functionalities implemented here:
 *
 * - asynchron/token based flow, with single value or per gate input token port
 * 	with the necessary queue management
 *
 * - gate notifying: sensitivity implementation moves here, devuser can turn off
 * the notifying per port, so the execution logic will not enqueued by writing
 * the wire's value (gates doesn't need implement sensitivity)
 *
 * - debug functionalities: registering hooks before wire writing and before
 * 	gate notifying
 *
 * - on token management: if queue is full, block the writer gate, and
 * 	propagate back the blocking.
 *
 * 		two reason for back propagating the wire's reference:
 * 			-
 *
 *
 *
 *
 * TODO functionality:
 * 	- ignore repeated values (on/off)
 *
 * */
struct lxc_wire
{
	Signal type;
	int subtype;
	//TODO queue and single token for tokenless mode
	LxcValue current_value;

	char* ref_des;

	struct key_value** wire_debug_hooks;

	Tokenport* drivers;
	uint drivers_length;

	Tokenport* drivens;
	uint drivens_length;
};


#define DIRECTION_IN true
#define DIRECTION_OUT false

enum lxc_errno
{
	SUCCESS = 0,
	LXC_ERROR_ENTITY_OUT_OF_RANGE = -1,
	LXC_ERROR_TYPE_NOT_SUPPORTED = -2,
	LXC_ERROR_PORT_IS_IN_USE = -3,
	LXC_ERROR_PORT_IS_ALREADY_FREE = -4,
	LXC_ERROR_BAD_CALL = -5,

	LXC_ERROR_ALREADY_HAS_DRIVER = -6,

	//tries to unwire a gate which is not registered in the wire drivees
	LXC_ERROR_NOT_CONNECTED_UNWIRING = -7,

	LXC_ERROR_GATECTL_BAD_CALL = -8,

	LXC_ERROR_INPUT_PORT_ALREADY_EXISTS = -9,
	LXC_ERROR_OUTPUT_PORT_ALREADY_EXISTS = -10,

	LXC_ERROR_NOTHING_CHANGED = -11,

	LXC_ERROR_PORT_DOESNOT_EXISTS = -12,

	LXC_ERROR_ILLEGAL_REQUEST = -13,

	LXC_ERROR_ENTITY_ALREADY_REGISTERED = -14,
	LXC_ERROR_ENTITY_NOT_FOUND = -15,

	LXC_ERROR_ENTITY_BY_NAME_ALREADY_REGISTERED = -16,

	LXC_ERROR_TYPE_CONVERSION_NOT_EXISTS = -17,

	LXC_ERROR_CORRUPTION = -18,

	LXC_ERROR_ILLEGAL_NAME = -100,

	LXC_ERROR_ENTITY_NOT_EXISTS = -104,

	LXC_ERROR_TOO_MANY_TYPES = -1024,

	LXC_ERROR_LIBRARY_CANT_OPEN = -1025,
	LXC_ERROR_LIBRARY_SYMBOL_NOT_FOUND = -1026,
	LXC_ERROR_RESOURCE_BUSY = -1027,





};

enum library_operation
{
	library_before_load,


	library_after_loaded,

};

enum gate_event_type
{
	custom_event,

	gate_enabled_disabled,

	input_wire_changed,
	output_wire_changed,

	input_portset_changed,
	output_portset_changed,

	attribute_changed,
	attribute_set_changed,

	gate_placed,
	gate_removed,




};

struct gate_event
{
	Gate gate_of_event;

	enum gate_event_type event_type;
	int event_subtype;

	int additional_data_identifier;
	void* previous_additional_data;
	void* additional_data;

};

struct event_listener;

/**
 * Structure stores the basic functions of the logic gate.
 *
 * Input and output types may vary trough the time.
 *
 * */
struct lxc_gate_behavior
{
	//returns the gate name like: nand, nor, struct, ao_out, v4l_input etc.
	const char* gate_name;

	Gate (*create)(const struct lxc_gate_behavior* this_behavoir);

	//destroys the logx gate instance even if is in use, so framework should
	//care about to take away all resource before this method called
	//eg.: close FDs, unwire input and outputs, unreference outside created but
	//internally used LxcValues, free allocated memory.
	void (*destroy)(Gate instance);

	//wire:

	//tries to copy the supported input type into the given array
	//see `destination arrays` how does it do.

	//if you implement a gate supports more than MAX_GATE_IO_TYPE_COUNT
	//increment the value and recompile core utilities.
	int (*get_input_types)(Gate instance, Signal* arr, int* subtype, uint max_length);

	//get the input user friandly name
	const char* (*get_input_label)(Gate instance, Signal signal, int subtype, uint index);

	//get the max index of the specified type, returns negative value if
	//type not supported
	int (*get_input_max_index)(Gate instance, Signal type, int subtype);

	//returns the wire of the specified type and input. returns null if
	//type not supported or port is not wired
	Tokenport (*get_input_wire)(Gate instance, Signal type, int subtype, uint index);

	//\\tries to wire the input. Signal parameter is redundant, if wire is not null
	//\\it will be silently ignored, if wire is null gate unwire the input specified
	//\\by type and index.

	//simply set the wire in the internal data structure specified by signal
	int (*wire_input)(Gate instance, Signal sig, int subtype, Tokenport port, uint index);

	//here notified if an input value changed, you can decide do you want to
	//care about (sensitivity), if you want to execute them without any
	//modification call instance->execution_behavior(instance,type,value,index)
	//removed function: void (*input_value_changed)(Gate instance, Signal type, LxcValue value, uint index);

	//execute the gate specific function
	//this will be called by the input_value_changed function as it implemented
	//and by the framework if:
	// a new input wired,
	// an input unwired,
	// a new output wired
	// gate enabled,
	//
	//TODO etc.
	//TODO precise doc how does it do.
	void (*execute)(Gate instance, Signal type, int subtype, LxcValue value, uint index);


	int (*get_output_types)(Gate instance, Signal* arr, int* subtype, uint max_length);

	const char* (*get_output_label)(Gate instance, Signal signal, int subtype, uint index);

	//get the max index of the specified type, returns negative value if
	//type not supported
	int (*get_output_max_index)(Gate instance, Signal type, int subtype);

	//returns the wire of the specified type and output. returns null if
	//type not supported or port is not wired
	Wire (*get_output_wire)(Gate instance, Signal type, int subtype, uint index);

	//\\tries to wire the input. Signal parameter is redundant if wire is not null
	//\\and it will be silently ignored, if wire is null gate unwires the input specified
	//\\by the signal type and index.

	//simply set the wire in the internal data structure specified by signal
	int (*wire_output)(Gate instance, Signal type, int subtype, Wire wire, uint port);

	//TODO get_lock_for_execution
	//a structure of function pointers contains the data for locking
	//mechanism

	//prop: set,get enumerate, label, description
	//
	int (*enumerate_properties)(Gate instance, const char** arr, uint max_length);

	const char* (*get_property_label)(Gate instance, const char* property);

	const char* (*get_property_description)(Gate instance, const char* property);

	int (*get_property_value)(Gate instance, const char* property, char* dst, uint max_length);

	//returns zero if property successfilly modified, return negative required length if error
	//buffer is too short to write error description. returns positive value if error ocurred and
	//error description successfully written back.
	int (*set_property)(Gate instance, const char* property, const char* value, char* err, uint max_length);

	//functionality like ioctl
	int (*gatectl)(Gate instance, unsigned long request, ...);

	//TODO save/restore with DataReprez

	//TODO etc data (library, documentation url, graphical symbol (SVG))

	int (*library_operation)(enum library_operation, const char** errors, int errors_max_length);

	//array_pnt library paths: path1, path2, NULL,
	const char*** paths;


	//this method should return the user utility descriptor's url.
	//all other method responsible to manage, setup and make the gate functional.
	//The graphical user interface is another huge task of a proper and - peasant
	//to use - implementation of the project with full of external resources
	//(gate appearance view, help article). i think, there's no place here for
	//the GUI servant functions, the behavior essentially should be the
	//functional part of a gate.
	//
	//source/doc
	//graphical symbol
	//gatectl / property utility
	//
	const char* (*get_user_utility_url)();



};

struct lxc_instance
{
	const struct lxc_gate_behavior* behavior;
	//notify gate for input value modification.
	//this is independ from the execution, notifying the gate
	//not always means, control flow will execute the gate specific opeartion.

	//TODO framework data eg locks
	char enabled;

	char* ref_des;

	Circuit owner_circuit;

	//in this method you can implement signal sensitivity.
	//or in this method you can store the given LxcVale and execure later or
	//other way than with the current control flow or thread.
	void (*execution_behavior)(Gate instance, Signal type, int subtype, LxcValue value, uint index);



	//long_lock
	/*
	 * queue for execution (even if direct execution applied, asynchron task
	 *  also should be enqueued)
	 */
	struct event_listener** event_listeners;

	//atomic reference blocked_by (on token wire value overproducing)
};

struct lxc_constant_value
{
	const char* name;
	const LxcValue value;
};

/************************ Framework system events *****************************/

/**
 * Events propagated to the gates
 * */
enum lxc_system_event_type
{
	//note: Never change enable and disable event position
	//some condition assume that disable is 0 and enable is the 1
	//system event
	system_event_gate_disabled,
	system_event_gate_enabled,

	/**
	 * system_event_input_wire_added,
	 *
	 * this case is unnecessary, because gate will be notified by the
	 * natural usage of input_value_changed() method.
	 */

	/**
	 * system_event_input_wire_removed,
	 *
	 * and in this case input_value_changed will called with NULL value.
	 * It is natural to bring a Wire to NULL value, but before notification
	 * we remove the input wire.
	 */

	system_event_output_wire_added,
	system_event_output_wire_removed,


	system_event_property_modified,
};

struct lxc_system_event
{
	enum lxc_system_event_type event_type;
	Signal signal;
	int subtype;
	int index;
	const char* name;
};


/************************ Subcircuit definitions ******************************/

struct circuit
{
	Gate* gates;
	Wire* wires;

	Wire* inputs;
	Wire* outputs;

	/**
	 * if this circuit is a circuit inside a gate, this event listener can
	 * be specified to relay events to the circuit, owned this gate (aka circuit)
	 * */
	struct event_listener** listeners;

	Circuit parent;
	Circuit* childs;
	char* name;
};

void lxc_destroy_simple_free(Gate instance);
void lxc_init_instance(Gate instance, const struct lxc_gate_behavior* behavior);

Circuit lxc_create_circuit();
int lxc_add_gate_to_circuit(Circuit, Gate);

void lxc_destroy_circuit(Circuit);


/********************** LOADABLE LIBRARY DEFINITIONS **************************/
struct lxc_loadable_library;


struct lxc_loadable_library
{
	int (*library_operation)(enum library_operation, const char** error, int max_length);

	//NULL terminated array (array_pnt)
	const struct lxc_gate_behavior** gates;
	//Signals used by the library
	Signal* signals;
	const struct lxc_constant_value** constants;

	//TODO struct types

	//TODO license
	//TODO URL info source/doc/overview


};

/********************* Event and listeners for the outside ********************/

struct event_listener
{
	void* owner;

	//gate_event_type mask
	int listen_mask;

	void (*event_listener)(struct gate_event*);
};

//bool can_propagate_event(struct gate_event_type);

void propagate_event(Gate, struct gate_event*);

void try_notify_gate_enabled_disabled(Gate);

void try_notify_attriubte_change(Gate);














#endif /* LOGX_H_ */

/*
 	copy this template to initalize your type:

		.get_gate_name = ,
		.create = ,
		.destroy = ,
		.get_input_types = ,
		.get_input_label = ,
		.get_input_max_index = ,
		.get_input_wire = ,
		.wire_input = ,
		.execute = ,
		.get_output_types = ,
		.get_output_label = ,
		.get_output_max_index = ,
		.get_output_wire = ,
		.wire_output = ,
		.enumerate_properties = ,
		.get_property_label = ,
		.get_property_description = ,
		.get_property_value = ,
		.set_property = ,
*/

