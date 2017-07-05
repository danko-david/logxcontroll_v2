
#ifndef USECASES_H_
#define USECASES_H_

#include "core/logxcontroll.h"

struct case_option
{
	const char* name;
	void (*funct)();
};

void register_option
(
	struct case_option*** opts,
	const char* name,
	void (*funct)
);

void computerphile_sort(int argc, char **argv, int start_from);
void type_sizes(int argc, char **argv, int start_from);
void functional_wrap(int argc, char **argv, int start_from);

void fw_regex(int argc, char **argv, int start_from);

void print_entities(int argc, char **argv, int start_from);

void builds_custom_experiment(int argc, char **argv, int start_from);

#endif /*USECASES_H_*/
