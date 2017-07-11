
#include "core/logxcontroll.h"
//static void func_fd_accept(file_descriptor** l_fd, LxcValue** o_fd)
#ifndef WITHOUT_PCRE
static struct compiled_regex GATE_DEF;
static struct compiled_regex GATE_PROP_MATCH;
static struct compiled_regex WIRING;

static void check_regex_error(const char* regex_name, int ret, const char* msg)
{
	if(0 != ret)
	{
		printf
		(
			"Internal regex \"%s\" compilation error: %s\n",
			regex_name,
			msg
		);
		exit(1);
	}
}

void ensure_circuit_builder_regexes_initialized()
{
	if(NULL != GATE_DEF.regex)
	{
		return;
	}

	const char* err;
	int ret = regex_compile
	(
		&GATE_DEF,
		"(?P<gate_type>[a-z_][a-z0-9]*)\\s*\\[(?P<ref_des>[a-z_][a-z0-9]*)\\]\\s*\\({(?P<props>.*)\\})?",
		PCRE_CASELESS,
		&err
	);
	check_regex_error("GATE_DEF", ret, err);

	//TODO


/*
	tcp_bind[bind]{prop=asd;prop2="as;df",prop3="df\"as\""}
	@8080 --bind_port--> bind.port

	socket_accept[accept]
	bind.fd ---> accept.accept

	http_wrap_context[http_ctx]
	accept.fd --> http_ctx.fd

	http_accept_request[http_accept]
	http_ctx.ctx --> http_accept.in_ctx

*/
};


void lxc_circuit_build_from_plain
(
	IOCircuit dst,
	const char* (*line_examiner)(void*),
	void* param
)
{
	ensure_circuit_builder_regexes_initialized();
	const char* line;
	while(NULL != (line = line_examiner(param)))
	{
		//TODO regex parser


	}
}
#endif

