/*
 * line_processor.c
 *
 *  Created on: 2017. 07. 15.
 *      Author: szupervigyor
 */

#include "core/logxcontroll.h"
//#include "experiment/lib/os_bind/os_bind.h"
/**
 *
 * 1 create a new file and fill at least 1000 line of string
 * 2 assemble the line processor network, enable, notify
 *
 * The point in this usecase is that no wire will fetch all the 1000 line of
 * text, if any wire exceed maximal token capacity... well, something should
 * happen ( :D ), loading all data eagerly is an objectionable solution,
 * because memory is limited.
 *
 * What's happens in this case is specified by the Wire's
 * wire_handler_logic.
 * 	- If we receiving a keep alive or status update signal,
 * we don't need all the values, but the last one, in this case lately coming
 * values may be dropped.
 *	- But like in this case, we want to process all the values coming from a
 * stream, without losing any value. To prevent overproducting char_replace may
 * block the `read_all_line` Gate of if writef executes slower it may block
 * the `char_replace` Gate. In total, writef may block both char_replace and
 * read_all_line Gates and that will happens if we shrink intermediate wires
 * capacity that can't hold values in a distributed way.
 *
 * This trick involves the modification of wire_handler_logic and lxc_scheduler.
 *
 *
 *	/-------\	filename	/-------\	fd		/---------------\
 *  | const	| --------------| fopen	| ---------	| read_all_line	| ----->
 * 	\-------/		mode ---|		| -errno	|				|
 * 							\-------/			\---------------/
 *
 *		/---------------\  out_str	/-----------\
 *	>---|  char_replace	|-----------|  printf	|
 *		\---------------/			|	%s\n	|
 *						  			\-----------/
 *
 *	All of the gates is only used to create a testcase to can start to
 *	implmenet the required functions to this feature.
 *
 *	Another good idea (i think) is to externalize buffer used to read line,
 *	so you can set the buffer characteristict from the outside, can easyli
 *	reused... and might do some unexcepted things.
 *
 *	TODO support to check volitions:
 *	If the gate produce value too fast and values
 *	congestings on the wire, the system may apply hard
 * 	blocking.
 *
 * 	Soft blocking: the gate overproduce bust gets the
 * 	control back to the system and the system can blocking
 * 	softly, even if all inputs are preset, the system won't
 * 	enqueue the gate.
 *
 * 	What's if a gate getting crazy and producing massive
 * 	amount of values under one activity? Well, we can't
 * 	take problem from the implementation side: i can't
 * 	kill a running thread and re-enqueue later.
 *
 * 	What i can do is to suspend the thread, and continue
 * 	the execution if values consumed from the wire.
 * 	But that's comes with som problems:
 * 		- We can't simply disable/interrupt the gate in
 * 			this state
 * 		- Thread stucks with the gate.
 * 		- circuit can't go idle, therefore systam can't be
 * 			shutdown. (TODO force destroy circuit: wire can
 * 				set up to consume and drop all incoming
 * 				values so the gate may put the control back)
 * */

/********************************* Gate const *********************************/
struct const_gate_instance
{
	struct lxc_generic_porti_instance base;
	struct lxc_constant_value* constant_value;
};

static void* const_gate__access_property(Gate instance, const char* val)
{
	if(0 == strcmp("const", val))
	{
		struct const_gate_instance* g =
			(struct const_gate_instance*) instance;
		return &g->constant_value;
	}

	return NULL;
}

int const_gate__const_selector
(
	Gate instance,
	bool direction,
	void* addr,
	const char* name,
	const char* value,
	char* ret,
	int max_length
)
{
	if(0 != strcmp("const", name))
	{
		//fw should catch nonexsiting property calls.
		return LXC_ERROR_CORRUPTION;
	}

	if(DIRECTION_OUT == direction)
	{
		const char* rn = "";
		if(NULL != addr)
		{
			struct lxc_constant_value* cv = (struct lxc_constant_value*) addr;
			rn = cv->name;
		}

		safe_strcpy(ret, max_length, rn);
		return 0;
	}
	else
	{
		struct lxc_constant_value* new_val;

		struct const_gate_instance* g =
			(struct const_gate_instance*) instance;

		//wipe value, but we don't release the port
		//(next value might have the same type)
		if(0 == strcmp(value, ""))
		{
			new_val = NULL;
			g->constant_value = new_val;
			return 0;
		}

		new_val = lxc_get_constant_by_name(value);
		if(NULL == new_val)
		{
			return LXC_ERROR_ENTITY_NOT_FOUND;
		}

		struct lxc_full_signal_type* full = NULL;

		if(NULL !=g->base.output_ports.managed_types)
		{
			full = &g->base.output_ports.managed_types[0];
		}

		if(NULL != full)
		{
			//if compatibe, we dont care it's wired or not.
			if
			(
				full->signal == new_val->value->type
			||
				full->subtype == new_val->value->subtype_info
			)
			{
				g->constant_value = new_val;
				return 0;
			}

			Wire w = NULL;

			if(0 != g->base.outputs_length)
			{
				w = g->base.outputs[0];
			}

			//if already created, check that comaptible.
			if(NULL != w)
			{
				//is compatible?
				if
				(
					g->base.outputs[0]->type != new_val->value->type
				||
					g->base.outputs[0]->subtype !=
						new_val->value->subtype_info
				)
				{
					return LXC_ERROR_TYPE_NOT_SUPPORTED;
				}
			}
			else
			{
				//remove the port, new will be created
				lxc_port_remove_port
				(
					&g->base.output_ports,
					full->signal,
					full->subtype,
					0
				);
			}
		}

		//create output
		lxc_port_unchecked_add_new_port
		(
			&g->base.output_ports,
			"const",
			new_val->value->type,
			new_val->value->subtype_info,
			NULL
		);

		g->constant_value = new_val;
		return 0;
	}
}

void const__exec
(
	Gate instance,
	Signal type,
	int subtype,
	LxcValue value,
	uint index
)
{
	struct const_gate_instance* g =
				(struct const_gate_instance*) instance;

	struct lxc_constant_value* val = g->constant_value;
	Wire w = g->base.outputs[0];
	if(NULL != val && NULL != w)
	{
		lxc_drive_wire_value
		(
			NULL,
			0,
			w,
			val->value
		);
	}
}

static Behavior assemble_const_gate()
{
	struct lxc_generic_porti_propb_behavior* ret =
		malloc(sizeof(struct lxc_generic_porti_propb_behavior));

	lxc_init_from_prototype
	(
		ret,
		sizeof(struct lxc_generic_porti_propb_behavior),
		&lxc_generic_porti_propb_prototype,
		sizeof(struct lxc_generic_porti_propb_behavior)
	);

	ret->base.base.gate_name = "const";
	ret->base.instance_memory_size = sizeof(struct const_gate_instance);
	ret->properties.access_property = const_gate__access_property;

	lxc_add_property
	(
		&ret->properties,
		"const",
		"Constant value name",
		"Constant value from the pool that will be produced on the output",
		"",
		const_gate__const_selector
	);

	ret->base.base.execute = const__exec;

	return (Behavior) ret;
}

/********************************* Gate fopen *********************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//TODO os_service
const struct lxc_signal_type lxc_signal_fd =
{
	.name = "fd",
};

static void gate_open(void** params, LxcValue* ret)
{
	char* name = *((char**)params[0]);
	//int mode = *((int*)params[1]);
	int fd = open(name, O_RDONLY, 0);
	if(fd < 0)
	{
		LxcValue val = lxc_create_primitive_value(&lxc_signal_int);
		*((int*)lxc_get_value(val)) = errno;
		ret[1] = val;
	}
	else
	{
		//ret[0] = lxc_os_support_wrap_fd(fd);
		//it's a simplified implementation os_service comes later
		LxcValue val = lxc_create_primitive_value(&lxc_signal_fd);
		*((int*)lxc_get_value(val)) = fd;
		ret[0] = val;
	}
}

Behavior assemble_fopen()
{
	Signal s_int = &lxc_signal_int;
	return (Behavior) lxc_generic_shorthand_func_wrap
	(
		"fopen",
		gate_open,
		"filename", &lxc_signal_string, 0,
		NULL,
		"fd", &lxc_signal_fd, 0,
		NULL
	);
}

/****************************** Gate read lines *******************************/

#define MAX_LINE_LENGTH 20000

static int read_into_buffer_single(int fd, char** buff, int* ep)
{
	//hard wired... it's advised to set as property (max line length).
	if(NULL == *buff)
	{
		*buff = malloc(MAX_LINE_LENGTH+1);
	}

	int cep = *ep;

	if(MAX_LINE_LENGTH - cep > 0)
	{

		int n = read(fd, *buff+cep, MAX_LINE_LENGTH-cep);
		if(n > 0)
		{
			*ep += n;
		}
		return n;
	}

	return 0;
}

static char* pop_line_from_buffer(char** buff, int* ep)
{
	int end = *ep;

	int i=0;
	bool r = false;
	char* b = *buff;
	for(;i<=end;++i)
	{
		char c = b[i];
		switch(c)
		{
		case '\r':
			if(r)
			{
				goto pop;//previous was also a \r
			}
			else
			{
				r = true;
			}
			break;
		case '\n': goto pop;//catch last \n (pass trought)
		default: if(r) goto pop;//if prev was \r and now any other
		}
	}

	if(!r && i > end)
	{
		//if buffer is full
		if(end == MAX_LINE_LENGTH)
		{
			//pop the whole line
			*buff[MAX_LINE_LENGTH] = 0;
			*ep = 0;
			return strdup(*buff);
		}
		return NULL;
	}

	pop:
	if(r)
	{
		--i;
	}

	b[i] = 0;
	char* ret = strdup(b);

	++i;

	if(r)
	{
		++i;
	}

	int e = 0;
	for(;e<=end;++e,++i)
	{
		b[e] = b[i];
	}
	*ep = e;

	return ret;
}

#include "sys/select.h"

struct gate_instance_read_all_line
{
	struct lxc_generic_portb_instance base;
	char* buffer;
	int ep;
};

struct t_gate_fd
{
	struct gate_instance_read_all_line* gate;
	LxcValue fd;
};


static void do_wait_event(void* exec_param)
{
	struct t_gate_fd* param = (struct t_gate_fd*) exec_param;

	int* pfd = ((int*)lxc_get_value(param->fd));
	int fd = *pfd;

	for(;;)
	{
		fd_set fd_in, fd_out;
		struct timeval tv;

		FD_ZERO(&fd_in);
		FD_ZERO(&fd_out);

		FD_SET(fd, &fd_in);

		// Wait up to 10 seconds
		tv.tv_sec = 10;
		tv.tv_usec = 0;

		// Call the select
		int ret = select( fd + 1, &fd_in, &fd_out, NULL, &tv );

		// Check if select actually succeed
		if(ret == 0)
		{
			continue;
		}

		//TODO what if it's is blocked? (just add as pending call)

		LxcValue sys = lxc_create_system_event
		(
			system_event_wait_state_event,
			&lxc_signal_system,
			0,
			0,
			param->fd
		);

		param->gate->base.base.behavior->execute
		(
			(Gate) param->gate,
			&lxc_signal_system,
			0,
			sys,
			0
		);

		lxc_unreference_value(sys);
		lxc_unreference_value(param->fd);
		free(param);
		return;
	}
}

//selecting single file descriptor, nice job! (Y)
static void rough_select
(
	struct gate_instance_read_all_line* gate,
	LxcValue read_fd
)
{
	lxc_reference_value(read_fd);

	struct t_gate_fd* param = malloc(sizeof(struct t_gate_fd));
	param->gate = gate;
	param->fd = read_fd;
	int status = lxc_crutch_wp_exec(do_wait_event, (void*) param);
	if(0 != status)
	{
		lxc_unreference_value(param->fd);
		free(param);
	}
}

static void read_all_line__exec
(
	Gate instance,
	Signal type,
	int subtype,
	LxcValue value,
	uint index
)
{
	//TODO TEST_ASSERT_EQUAL(gs_busy, instance->state);

	struct gate_instance_read_all_line* gate =
		(struct gate_instance_read_all_line*) instance;

	LxcValue read_fd = NULL;

	if(&lxc_signal_system == type)
	{
		struct lxc_system_event* sys =
			(struct lxc_system_event*) lxc_get_value(value);

		if(system_event_wait_state_event == sys->event_type)
		{
			read_fd = sys->user_data;

			//read and pop
			int read = read_into_buffer_single
			(
				*((int*)lxc_get_value(read_fd)),
				&gate->buffer,
				&gate->ep
			);

			if(read < 0)
			{
				Wire out = gate->base.outputs[1];
				if(NULL != out)
				{
					//TODO error
				}

				lxc_unreference_value(read_fd);
				return;//go idle
			}
			else
			{
				Wire out = gate->base.outputs[0];
				for(;;)
				{
					/**
					 * TODO support to check volitions:
					 *  SEE up in the descrption (Soft and hard block)
					 */


					char* line = pop_line_from_buffer(&gate->buffer, &gate->ep);
					if(NULL == line)
					{
						break;
					}

					if(NULL == out)
					{
						free(line);
					}
					else
					{
						//XXX an experiment: create primitive value
						//with a given size after the last entry in the struct,
						//and poiner set to the start of it. (of course we need
						//a pop function that allocates the value at the time
						//it's determined the string's length (how that small
						//thing influence size and speed?)
						LxcValue val =
								lxc_create_primitive_value(&lxc_signal_string);

						*((char**)lxc_get_value(val)) = line;
						lxc_drive_wire_value(instance, 0, out, val);
						lxc_unreference_value(val);
					}
				}
			}
		}
	}
	else
	{
		Tokenport in = gate->base.inputs[0];
		if(NULL == in)
		{
			return;
		}
		read_fd = lxc_get_token_value(in);
		if(NULL == read_fd)
		{
			return;
		}
		else
		{
			lxc_absorb_token(in);
		}
	}

	if(NULL == read_fd)
	{
		return;
	}

	//TODO notifiecation of this request may be delayed but in a nonblocking way
	//(don't let the gate's lock to block the asyncronous notification,
	//and therefore block the logic behind)
	rough_select(gate, read_fd);

	//TODO set interrupting
	//plain method for now
	instance->state = gs_waiting;

	//TODO implementing wait state, interrupting, recall
}

static Behavior assemble__read_all_line()
{
	struct lxc_generic_portb_behavior* ret =
		malloc(sizeof(struct lxc_generic_portb_behavior));

	lxc_init_from_prototype
	(
		ret,
		sizeof(struct lxc_generic_portb_behavior),
		&lxc_generic_portb_prototype,
		sizeof(struct lxc_generic_portb_behavior)
	);

	ret->base.gate_name = "read_all_line";
	ret->instance_memory_size = sizeof(struct gate_instance_read_all_line);

	lxc_port_unchecked_add_new_port
	(
		&ret->input_ports,
		"fd",
		&lxc_signal_fd,
		0,
		NULL
	);

	lxc_port_unchecked_add_new_port
	(
		&ret->output_ports,
		"line",
		&lxc_signal_string,
		0,
		NULL
	);

	lxc_port_unchecked_add_new_port
	(
		&ret->output_ports,
		"error",
		&lxc_signal_fd,
		0,
		NULL
	);

	ret->base.execute = read_all_line__exec;

	return (Behavior) ret;
}

/**************************** Gate char replace  ******************************/

static void gate_replace(void** params, LxcValue* ret)
{
	char* in = *((char**)params[0]);
	int length = strlen(in);
	char* out = malloc(length+1);
	memcpy(out, in, length+1);

	int i=0;
	for(;i<length;++i)
	{
		if('a' == out[i])
		{
			out[i] = 'b';
		}
	}

	LxcValue val = lxc_create_primitive_value(&lxc_signal_string);
	*((char**)lxc_get_value(val)) = out;
	ret[0] = val;
}

Behavior assemble_char_replace()
{
	return (Behavior) lxc_generic_shorthand_func_wrap
	(
		"char_replace",
		gate_replace,
		"in", &lxc_signal_string, 0,
		NULL,
		"out", &lxc_signal_string, 0,
		NULL
	);
}

/********************************* printf *************************************/

static void gate_printf(void** params, LxcValue* ret)
{
	char* in = *((char**)params[0]);
	printf("given line: %s\n", in);
}

Behavior assemble_printf()
{
	Signal s_int = &lxc_signal_int;
	return (Behavior) lxc_generic_shorthand_func_wrap
	(
		"printf",
		gate_printf,
		"in", &lxc_signal_string, 0,
		NULL,
		NULL
	);
}

/********************************** Main **************************************/

static void register_line_processor_gates()
{
	lxc_register_gate(assemble_const_gate());
	lxc_register_gate(assemble_fopen());
	lxc_register_gate(assemble__read_all_line());
	lxc_register_gate(assemble_char_replace());
	lxc_register_gate(assemble_printf());
}

Gate create_gate_by_name_or_fail
(
	IOCircuit circ,
	const char* behavior_name,
	const char* ref_des
)
{
	Gate g = lxc_gate_create_by_name(behavior_name);
	if(NULL == g)
	{
		printf
		(
			"Gate behavior with name \"%s\" doesn't exists.\n",
			behavior_name
		);
		exit(1);
	}

	int status = lxc_gate_set_refdes(g, ref_des);
	if(0 != status)
	{
		printf("Can't set gate refdes: %d, %d\n", behavior_name, ref_des);
		exit(2);
	}

	status = lxc_circuit_add_gate(circ, g);

	if(0 != status)
	{
		printf("Can't add gate to circuit: %d, %d\n", behavior_name, ref_des);
		exit(2);
	}

	return g;
}

void error_exit(int error)
{
	if(0 != error)
	{
		printf("Error raised: %d\n", error);
		print_stack_trace_then_terminalte();
	}
}

static struct lxc_constant_value const_testfile_name;

static struct lxc_constant_value* init_filename_const
(
	const char* name,
	const char* value
)
{
	const_testfile_name.name = name;
	LxcValue v = lxc_create_primitive_value(&lxc_signal_string);
	*((const char**)lxc_get_value(v)) = value;
	LxcValue* dst = ((LxcValue)&const_testfile_name.value);
	*dst = v;
	return &const_testfile_name;
}

static void notify_gate_test(Gate g)
{
	g->execution_behavior(g, NULL, 0, NULL, 0);
}


void line_processor(int argc, char **argv, int start_from)
{
	logxcontroll_init_environment();
	register_line_processor_gates();

	const char* const_name = "line_processor_test_filename";
	const char* filename = "67678678_my_testfile_4765233g";

	{
		int status = lxc_register_constant_value
		(
			init_filename_const(const_name, filename)
		);

		TEST_ASSERT_EQUAL(0, status);
	}

	int test_line_count = 850;

	unlink(filename);
	//generate file
	{
		int fd = open(filename, O_CREAT | O_RDWR, S_IRWXU);
		if(fd < 0)
		{
			perror("open file");
			exit(5);
		}

		char buf[40];
		{
			int i = 0;
			for(;i<test_line_count;++i)
			{
				int len = rand() % 40;
				int c = 0;
				for(;c < len;++c)
				{
					buf[c] = 'a' + (rand() % 8);
				}

				buf[len] = '\n';

				TEST_ASSERT_TRUE
				(
					0 < write(fd, buf, len+1)
				);
			}
		}

		fsync(fd);

		TEST_ASSERT_TRUE(0 == close(fd));
	}

	IOCircuit circ = lxc_circuit_create();

	Gate g_const = create_gate_by_name_or_fail(circ, "const", "const");
	{
		int ret = lxc_gate_set_property_value
		(
			g_const,
			"const",
			const_name,
			NULL,
			-1
		);
		TEST_ASSERT_EQUAL(0, ret);
	}

	create_gate_by_name_or_fail(circ, "fopen", "fopen");
	create_gate_by_name_or_fail(circ, "read_all_line", "read_all_line");
	create_gate_by_name_or_fail(circ, "char_replace", "char_replace");
	create_gate_by_name_or_fail(circ, "printf", "printf");

	TEST_ASSERT_EQUAL
	(
		0,
		lxc_circuit_wire_ports_together
		(
			circ,
			"const", "const",
			"filename",
			"fopen", "filename",
			true
		)
	);

	TEST_ASSERT_EQUAL
	(
		0,
		lxc_circuit_wire_ports_together
		(
			circ,
			"fopen", "fd",
			"opened_file",
			"read_all_line", "fd",
			true
		)
	);

	TEST_ASSERT_EQUAL
	(
		0,
		lxc_circuit_wire_ports_together
		(
			circ,
			"read_all_line", "line",
			"readed_line",
			"char_replace", "in",
			true
		)
	);

	TEST_ASSERT_EQUAL
	(
		0,
		lxc_circuit_wire_ports_together
		(
			circ,
			"char_replace", "out",
			"replaced",
			"printf", "in",
			true
		)
	);

	//lxc_dbg_print_dot_graph(circ);

	lxc_circuit_set_all_gate_enable(circ, true);

	notify_gate_test(g_const);

	logxcontroll_destroy_environment();

	//TODO unlink file
	unlink(filename);
}
