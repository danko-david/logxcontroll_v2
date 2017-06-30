
#include "experiment/usecases/usecases.h"


/****************************** usecase register ******************************/

struct case_option
{
	const char* name;
	void (*funct)();
};

static struct case_option** OPTS = NULL;

void register_option(const char* name, void (*funct))
{
	struct case_option* add = malloc(sizeof(struct case_option));
	add->name = name;
	add->funct = funct;
	array_pnt_append_element
	(
		(void***) &OPTS,
		(void*)add
	);
}

int main(int argc, char **argv)
{
	//register_option("", );
	register_option("computerphile_sort", computerphile_sort);


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

