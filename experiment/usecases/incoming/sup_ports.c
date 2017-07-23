/*
 * sup_ports.c
 *
 *  Created on: 2017. júl. 15.
 *      Author: szupervigyor
 */

/**
 * Supplementary port impl. and test:
 *
 * 	Supplementry port are an extension available for gate:
 *
 * 	Imagine this situation:
 *
 * 	We want to read data from a file descriptor
 *
 * 						/-----------\
 * 			fd		----|	 read	|---- data
 * 			bufsize ----|	 Gate	|---- errno
 * 						\-----------/
 *
 *			How to report which file descriptor raised an error,
 *			if this gate produces only `data` on the output on success
 *			or `errno` when error occurs.
 *
 *			We want to gate product something like this:
 *			Tupe<fd, errno>
 *
 *	How to solve this problem?
 *		Skip: - You might wire up a merger gate right after this `read` gate,
 *			mergimg fd and errno together into a Tupe.
 *			but with any type of wire.
 *
 *			in this case: yout need a special type of wire: token flow managed
 *				trought Tokenports and wire_handler_logic, so you
 *				  a wire accepts single token per time
 *				 one of the Tokenport (the read's) can consume the token and
 *				 "request" (let previous gate, that provied the fd to can
 *				 activitate and place a new value on the wire)
 *				 But the another tokenport can read only the current value of
 *				 the one valued wire, consumption as no effect and wire will be
 *				 automatically overwritten when the master Tokenport (read.fd)
 *				 consumpts.
 *
 *				The merger binds values togethet only if both value available
 *					at the same activisation, otherwise: if only fd available
 *					at one time, merging values cannot be done, so it's
 *					triggeder only if fd and errno available at the same time.
 *
 *				 wire up any wire between errno and the merger's errno input.
 *
 *				 this solution is a very gate implmenetation and timing depend
 *				 solution:
 *					it's gonna work perfectly if gate implementation comsumpts
 *					token only after wiring the output wire `data` (it's not the
 *					desired behavior) and you use everywhere direct execution
 *					(which will be modified in future to resist
 *					recursion/loopbacks)
 *
 *				In other cases fd may be overwritten after `read` gate consumpts
 *					the token, merging bad values together.
 *
 *
 *		Another idea is to wrap this gates into a subcircuit/compaund gate.
 *		The new gate has `fd` and `bufsize` input and `data` , Tupe<fd,errno>
 *		outout after activisation of this node data or Tupe<fd, errno> produced
 *		and gates and wires are resetted in (no no value remains on the wires
 *		inside)
 *
 *		That's gonna be a good alternative.
 *
 *	Supplementary ports are prvided to the user side, enables to expand
 *		out of box gates with extra ports that can examine values used/produced
 *		on a single gate activison. So our gates with supplementary port looks
 *		like this:
 *
 *
 *						/-----------\
 * 			fd		----|	 read	|---- data
 * 			bufsize ----|	 Gate	|---- errno
 *						\-----------/
 *							|	|
 *							|	|
 *							|	\----- fd
 *							|
 *							\--------- errno
 *
 *		TODO: describe that this doesn't solves the problem only if this
 *			mechanism can merge values in activision
 *			Whats' about multiple value production on single activity, if you
 *			want to pair values (spec first/last)
 * */

