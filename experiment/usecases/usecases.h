
#ifndef USECASES_H_
#define USECASES_H_

#include "core/logxcontroll.h"

struct case_option
{
	char* name;
	void (*funct)();
};

void options_register
(
	struct case_option*** opts,
	const char* name,
	void (*funct)
);

void options_release(struct case_option** opts);


#ifdef INCLUDE_NOVAPROVA
	void novaprova(int argc, char **argv, int start_from);
#endif

void computerphile_sort(int argc, char **argv, int start_from);
void type_sizes(int argc, char **argv, int start_from);
void functional_wrap(int argc, char **argv, int start_from);

void print_entities(int argc, char **argv, int start_from);

void fw_regex(int argc, char **argv, int start_from);

void oscillator(int argc, char **argv, int start_from);

void prell(int argc, char **argv, int start_from);

void line_processor(int argc, char **argv, int start_from);

void builds_custom_experiment(int argc, char **argv, int start_from);

#endif /*USECASES_H_*/
