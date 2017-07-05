
#include "experiment/usecases/usecases.h"

/****************************** usecase register ******************************/

void register_option
(
	struct case_option*** opts,
	const char* name,
	void (*funct)
)
{
	struct case_option* add = malloc(sizeof(struct case_option));
	add->name = name;
	add->funct = funct;
	array_pnt_append_element
	(
		(void***) opts,
		(void*) add
	);
}

int main(int argc, char **argv)
{
	struct case_option** OPTS = NULL;

	register_option(&OPTS, "computerphile_sort", computerphile_sort);
	register_option(&OPTS, "type_sizes", type_sizes);
	register_option(&OPTS, "functional_wrap", functional_wrap);

	register_option(&OPTS, "print_entities", print_entities);


	register_option(&OPTS, "fw_regex", fw_regex);

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
	return 0;
}

