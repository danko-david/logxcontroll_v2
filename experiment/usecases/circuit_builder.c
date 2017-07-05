
#include "core/logxcontroll.h"
//static void func_fd_accept(file_descriptor** l_fd, LxcValue** o_fd)

static struct compiled_regex GATE_DEF;



void ensure_circuit_builder_regexes_initialized()
{
/*
	tcp_bind[bind]{}
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


