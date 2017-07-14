
#include "experiment/usecases/usecases.h"

/****************************** usecase register ******************************/

void options_register
(
	struct case_option*** opts,
	const char* name,
	void (*funct)
)
{
	struct case_option* add = malloc(sizeof(struct case_option));
	add->name = strdup(name);
	add->funct = funct;
	array_pnt_append_element
	(
		(void***) opts,
		(void*) add
	);
}

void options_release(struct case_option** opts)
{
	if(NULL == opts)
	{
		return;
	}

	int i=-1;
	while(NULL != opts[++i])
	{
		free(opts[i]->name);
		free(opts[i]);
	}

	free(opts);
}

int main(int argc, char **argv)
{
	struct case_option** OPTS = NULL;

	#ifdef INCLUDE_NOVAPROVA
		options_register(&OPTS, "novaprova", novaprova);
	#endif

	options_register(&OPTS, "computerphile_sort", computerphile_sort);
	options_register(&OPTS, "type_sizes", type_sizes);
	options_register(&OPTS, "functional_wrap", functional_wrap);
	options_register(&OPTS, "print_entities", print_entities);

	options_register(&OPTS, "oscillator", oscillator);

	#ifndef WITHOUT_PCRE
		options_register(&OPTS, "fw_regex", fw_regex);
	#endif
	//register_option("builds_custom_experiment", builds_custom_experiment);

	char* ref = NULL;
	if(argc > 1)
	{
		ref = argv[1];
		int i=-1;
		while(NULL != OPTS[++i])
		{
			if(0 == strcmp(ref, OPTS[i]->name))
			{
				logxcontroll_init_environment();
				OPTS[i]->funct(argc, argv, 2);
				logxcontroll_destroy_environment();
				options_release(OPTS);
				return 0;
			}
		}
	}

	printf("Given usecase: \"%s\" not found.\n", ref);
	printf("Available usecases:\n");
	{
		int i = -1;
		while(NULL != OPTS[++i])
		{
			printf("==> \"%s\"\n", OPTS[i]->name);
		}
	}
	options_release(OPTS);
	return 1;
}

