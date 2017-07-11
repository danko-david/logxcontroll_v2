
#include "core/logxcontroll.h"


static void libtree(int argc, char **argv, int start_from)
{
	lxc_dbg_print_library_tree(true);
}

void print_entities(int argc, char **argv, int start_from)
{
	struct case_option** OPTS = NULL;

	options_register(&OPTS, "libtree", libtree);

	char* ref = NULL;
	if(argc > 2)
	{
		ref = argv[2];
		int i=-1;
		while(NULL != OPTS[++i])
		{
			const char* name = OPTS[i]->name;
			if(0 == strcmp(ref, name))
			{
				OPTS[i]->funct(argc, argv, 3);
				return;
			}
		}
	}

	printf("Given printcase: \"%s\" not found.\n", ref);
	printf("Available printcase:\n");

	{
		int i = -1;
		while(NULL != OPTS[++i])
		{
			printf("==> \"%s\"\n", OPTS[i]->name);
		}
	}
}
