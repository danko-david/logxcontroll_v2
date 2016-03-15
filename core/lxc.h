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
 *		//TODO there is some alternatives for data propagation
 *		self execution must check for stack overflow before it's occurred.
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
 *		relative pointer.
 *
 *
 *
 *
 *
 * */
#include "core/logxcontroll.h"


#ifndef LOGX_H_
#define LOGX_H_


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
typedef struct lxc_port* Port;

struct lxc_cast_to
{
	Signal to;
	LxcValue (*cast_function)(LxcValue);
};

/**
 * Contains the necessary data about the signal type and framework utility.
 * */
struct lxc_signal_type
{
	/*
	 * The user friendly name of signal
	 */
	const char* name;

	/*
	 * Check signal equals to another.
	 */
	bool (*equals)(LxcValue, LxcValue);


	/**
	 * Map is array_pnt of key_value*
	 *
	 * A mapping used for casting value:
	 * Map<Signal, LxcValue (*cast_to)(LxcValue)>
	 */
	struct lxc_cast_to** cast_to;
};

//atomically increment the underlying value's reference count.
int lxc_reference_value(LxcValue value);

//atomically decrement the underlying value's reference count,
//if reference count is 0 (or less), resource will be freed and 0 return
//otherwise current reference count returned.
int lxc_unference_value(LxcValue value);

struct lxc_port
{
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

	//returns the underlying value exact copy.
	//for you can modify an incoming value (in  the aspect of a gate)
	//you may not modify directly the incoming value, first it must be cloned.
	//if the underlying value is a constant and you clone that,
	//the new value must work like a valueable value.
	LxcValue (*clone)(LxcValue dest);

	//returns the size of the value
	//if type is complex or array of element like,
	//the size is sum of all unique element.
	size_t (*size)(LxcValue value);

	//increment the reference counter with 1 and returns the
	//current reference count. If no reference counting applied return 1
	//or this function is null
	int (*reference)(LxcValue value);

	//decrements the reference counter and returns the current reference
	//count if no more reference used it should automatically free.
	int (*unreference)(LxcValue value);

	/**
	 * the data is always in the same structure, but can be aligned
	 *
	 * */
	void* (*data_address)(LxcValue value);
};

struct lxc_value
{
	Signal type;

	//type of the signal trouth some datatype may be same,
	//but same type of values may behaviors different.
	//some primitive values may be placed in a constant pool
	//text section calling free for this values may causes runtime breaking.
	const struct lxc_value_operation* operations;

	int refcount;
};

LxcValue lxc_create_generic_value(Signal,size_t);

#define LXC_GATE_MAX_IO_TYPE_COUNT 20

/**
 * Represents a wire, connect gates together.
 * A wire has driver(s) and driven gates.
 * */
struct lxc_wire
{
	Signal type;
	LxcValue current_value;

	Port* drivers;
	uint drivers_length;

	Port* drivens;
	uint drivens_length;
};


#define DIRECTION_IN true
#define DIRECTION_OUT false

enum lxc_errno
{
	SUCCESS = 0,
	LXC_ERROR_PORT_OUT_OF_RANGE = -1,
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

	LXC_ERROR_PROPERTY_ALREADY_REGISTERED = -14,
	LXC_ERROR_PROPERTY_NOT_FOUND = -15,

	LXC_ERROR_ILLEGAL_NAME = -100,

	LXC_ERROR_SIGNAL_ALREADY_REGISTERED = -101,
	LXC_ERROR_GATE_BY_NAME_ALREADY_REGISTERED = -102,
	LXC_ERROR_CONSTANT_VALUE_BY_NAME_ALREADY_REGISTERED = -103,

	LXC_ERROR_TYPE_CONVERSION_NOT_EXISTS = -104,

	LXC_ERROR_TOO_MANY_TYPES = -1024,

	LXC_ERROR_LIBRARY_SO_CANT_OPEN = -1025,
	LXC_ERROR_LIBRARY_SYMBOL_NOT_FOUND = -1026,





};

void lxc_drive_wire_value(Gate instance, uint out_index, Wire wire, LxcValue value);

int lxc_wire_gate_input(Signal type, Wire wire, Gate g, uint index);

int lxc_wire_gate_output(Signal type, Wire wire, Gate g, uint index);

Wire lxc_create_wire(Signal type);

/**
 * Wiring function:
 * 	wire_input and wire_output used for binding wires into/from the gate or
 * 	unwire a specified port
 *
 * example code:
 *
 	int wiring(Gate instance, Signal signal, Wire wire, int index)
 	{
 		//is the call legal?
 		if(NULL == signal && NULL == wire)
 			return WIRING_ERROR_BAD_CALL;

		if(NULL == signal)
		{
			signal = wire->type;
		}
		//signal is not null. But if the wire not null it is use the same type?
		else if(NULL != wire && signal != wire->type)
		{
			return WIRING_ERROR_BAD_CALL;
		}

		if(!is_im_support_this_type_of_signal(signal))
		{
			return WIRING_ERROR_TYPE_NOT_SUPPORTED;
		}

		if(!is_wire_by_type__index_is_in_range(signal, index))
		{
			return WIRING_ERROR_PORT_OUT_OF_RANGE;
		}

		//now user tries to unwire with specified index
		if(NULL == wire)
		{
			if(is_target_wire_NULL(signal, index))
			{
				return WIRING_ERROR_PORT_IS_ALREADY_FREE;
			}

			set_target_wire(signal, index, NULL);
			return 0;
		}
		else
		{
			if(!is_target_wire_NULL(signal, index))
			{
				return WIRING_ERROR_PORT_IS_IN_USE;
			}

			set_target_wire(signal, index, wire);
			return 0;
		}
	}
 *
 * */


/**
 * destionation arrays:
 * 	functions prototyped like: int function(array** arr, int max_length);
 *	tries to copy the data to the given destination address.
 *
 *	some examples:
 *
 *	every time the item count is 5.
 *
 *	{
 *		struct item* arr[1];
 *		int ret = function(arr, 1);
 *		//ret = -5
 *		//no modification in arr memory region
 *	}
 *
 *	{
 *		struct item* arr[5];
 *		int ret = function(arr, 5);
 *		//ret = 5
 *		//all the data are copied.
 *	}
 *
 *	{
 *		struct item* arr[10];
 *		int ret = function(arr, 10);
 *		//ret = 5
 *		//all data copied and arr is terminated. (arr[6] = NULL)
 *	}
 *
 *	{
 *		int ret = function(NULL, 0);
 *		//ret = -5
 *		//get the size
 *	}
 *
 * */

enum library_operation
{
	library_before_load,


	library_after_loaded,

	//=======

	library_before_unload,

	library_after_unloaded,


	library_unload_caused_before_load_error,

};


/**
 * Structure stores the basic functions of the logic gate.
 *
 * Input and output types may vary trough the time.
 *
 * */
struct lxc_gate_behavior
{
	//returns the gate name like: nand, nor, struct, ao_out, v4l_input etc.
	const char* (*get_gate_name)(Gate);

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
	int (*get_input_types)(Gate instance, Signal* arr, uint max_length);

	//get the input user friandly name
	const char* (*get_input_label)(Gate instance, Signal signal, uint index);

	//get the max index of the specified type, returns negative value if
	//type not supported
	int (*get_input_max_index)(Gate instance, Signal type);

	//returns the wire of the specified type and input. returns null if
	//type not supported or port is not wired
	Wire (*get_input_wire)(Gate instance, Signal type, uint index);

	//\\tries to wire the input. Signal parameter is redundant, if wire is not null
	//\\it will be silently ignored, if wire is null gate unwire the input specified
	//\\by type and index.

	//simply set the wire in the internal data structure specified by signal
	int (*wire_input)(Gate instance, Signal signal, Wire wire, uint index);

	//here notified if an input value changed, you can decide do you want to
	//care about (sensitivity), if you want to execute them without any
	//modification call instance->execution_behavior(instance,type,value,index)
	void (*input_value_changed)(Gate instance, Signal type, LxcValue value, uint index);

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
	void (*execute)(Gate instance, Signal type, LxcValue value, uint index);


	int (*get_output_types)(Gate instance, Signal* arr, uint max_length);

	const char* (*get_output_label)(Gate instance, Signal signal, uint index);

	//get the max index of the specified type, returns negative value if
	//type not supported
	int (*get_output_max_index)(Gate instance, Signal type);

	//returns the wire of the specified type and output. returns null if
	//type not supported or port is not wired
	Wire (*get_output_wire)(Gate instance, Signal type, uint index);



	//\\tries to wire the input. Signal parameter is redundant if wire is not null
	//\\and it will be silently ignored, if wire is null gate unwires the input specified
	//\\by the signal type and index.

	//simply set the wire in the internal data structure specified by signal
	int (*wire_output)(Gate instance, Signal signal, Wire wire, uint index);

	//TODO get_lock_for_execution
	//a structure of function pointers contains the data for locking
	//mechanism

	//prop: set,get enumerate, label, description
	//
	int (*enumerate_properties)(Gate instance, const char** arr, uint max_length);

	const char* (*get_property_label)(Gate instance, char* property);

	const char* (*get_property_description)(Gate instance, char* property);

	int (*get_property_value)(Gate instance, char* property, char* dst, uint max_length);

	//returns zero if property successfilly modified, return negative required length if error
	//buffer is too short to write error description. returns positive value if error ocurred and
	//error description successfully written back.
	int (*set_property)(Gate instance, char* property, char* value, char* err, uint max_length);

	//functionality like ioctl
	int (*gatectl)(Gate instance, unsigned long request, ...);

	//TODO save/restore with DataReprez

	//TODO etc data (library, documentation url, graphical symbol (SVG))
};

LxcValue lxc_get_wire_value(Wire);
void* lxc_get_value(LxcValue);


void lxc_destroy_simple_free(Gate instance);

struct lxc_instance
{
	const struct lxc_gate_behavior* behavior;
	//notify gate for input value modification.
	//this is independ from the execution, notifying the gate
	//not always means, control flow will execute the gate specific opeartion.

	//TODO framework data eg locks

	//in this method you can implement signal sensitivity.
	//or in this method you can store the given LxcVale and execure later or
	//other way than with the current control flow or thread.
	void (*execution_behavior)(Gate instance, Signal type, LxcValue value, uint index);

	char enabled;
};

struct lxc_constant_value
{
	const char* name;
	LxcValue value;
};



typedef struct circuit* Circuit;
typedef struct iocircuit* IOCircuit;

struct circuit
{
	uint gates_count;
	Gate* gates;

	uint wires_count;
	Wire* wires;
};

Circuit lxc_create_circuit();
int lxc_add_gate_to_circuit(Circuit, Gate);

void lxc_destroy_circuit(Circuit);


struct iocircuit
{
	struct circuit base;

	uint inputs_count;
	Wire* inputs;

	uint outputs_count;
	Wire* outputs;
};

IOCircuit lxc_create_iocircuit();

/********************** LOADABLE LIBRARY DEFINITIONS **************************/
struct loadable_library;

void lxc_ll_relay_library_operation(enum library_operation, struct loadable_library*);

//TODO where to belong
struct detailed_gate_entry
{
	const struct lxc_gate_behavior* behavior;

	const char* generic_name;

	//array_pnt library paths path1, path2, NULL,
	const char*** paths;

	//source/doc
	//graphical symbol
	//gatectl / property utility
	//
};


struct loadable_library
{
	int (*library_operation)(enum library_operation, char* error, int max_length);

	//NULL terminated array (array_pnt)
	struct detailed_gate_entry** gates;
	//Signals used by the library
	Signal* signals;
	struct lxc_constant_value* constants;

	//TODO struct types

	//TODO license
	//TODO URL info source/doc/overview


};


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

