/*
 * accounting.c
 *
 *  Created on: 2016.02.15.
 *      Author: szupervigyor
 */

#include "core/logxcontroll.h"

Signal* REGISTERED_SIGNALS = NULL;
struct library_tree_node** ROOT_NODES = NULL;
struct lxc_gate_behavior** REGISTERED_BEHAVIORS = NULL;
struct lxc_constant_value** REGISTERED_CONSTANT_VALUES;

int lxc_register_gate(struct lxc_gate_behavior* entry)
{
	struct lxc_gate_behavior* in = get_gate_entry_by_name(entry->gate_name);
	if(NULL != in)
		return LXC_ERROR_ENTITY_BY_NAME_ALREADY_REGISTERED;

	array_pnt_append_element((void***) &REGISTERED_BEHAVIORS, (void*) entry);

	const char*** paths = entry->paths;
	if(NULL != paths)
	{
		for(int i=0;NULL != paths[i];++i)
		{
			struct library_tree_node* node = get_or_create_library_path(paths[i]);
			array_pnt_append_element((void***)&(node->gates), (void*)entry);
		}
	}

	return 0;
}

int lxc_load_library(const struct lxc_loadable_library* lib, const char** errors, int maxlength)
{
	if(NULL == lib)
	{
		array_fix_try_add_last_null(errors, maxlength, "Library reference is NULL.");
		return LXC_ERROR_BAD_CALL;
	}

	int (*libop)(enum library_operation, const char**, int) = lib->library_operation;

	if(NULL != libop)
	{
		int ret = libop(library_before_load, errors, maxlength);
		if(0 != ret)
		{
			int un = libop(library_unload_caused_before_load_error, errors, maxlength);
			return 0 != un?un:ret;
		}
		//successfull library preload
	}

	//load signals and gate behaviors
	struct lxc_gate_behavior** gates = lib->gates;
	if(NULL != gates)
	{
		for(int i=0;NULL != gates[i];++i)
		{
			int ret = lxc_register_gate(gates[i]);
			if(0 != ret)
			{
				array_fix_try_add_last_null(errors, maxlength, "Gate already registered.");
				array_fix_try_add_last_null(errors, maxlength, gates[i]->gate_name);
			}
		}
	}

	Signal* sigs = lib->signals;
	if(NULL != sigs)
	{
		for(int i=0;NULL != sigs[i];++i)
		{
			int ret = lxc_register_signal(sigs[i]);
			if(0 != ret)
			{
				array_fix_try_add_last_null(errors, maxlength, "Signal already registered.");
								array_fix_try_add_last_null(errors, maxlength, sigs[i]->name);
				//TODO unregister then unload
				return ret;
			}
		}
	}

	struct lxc_constant_value** consts = lib->constants;

	if(NULL != consts)
	{
		for(int i=0;NULL != consts[i];++i)
		{
			int ret = lxc_register_constant_value(consts[i]);
			if(0 != ret)
			{
				array_fix_try_add_last_null(errors, maxlength, "Constant already registered.");
				array_fix_try_add_last_null(errors, maxlength, consts[i]->name);
			}
		}
	}

	if(NULL != libop)
	{
		int ret = libop(library_after_loaded, errors, maxlength);
		if(0 != ret)
		{
			//unload all
			//notify library
			return ret;
		}
	}

	//TODO register constants

	return 0;
}

int lxc_register_constant_value(struct lxc_constant_value* val)
{
	if(NULL != REGISTERED_CONSTANT_VALUES)
	{
		for(int i=0;NULL != REGISTERED_CONSTANT_VALUES[i];++i)
			if(0 == strcmp(val->name, REGISTERED_CONSTANT_VALUES[i]->name))
			{
				return LXC_ERROR_ENTITY_ALREADY_REGISTERED;
			}
	}

	array_pnt_append_element((void***)&REGISTERED_CONSTANT_VALUES, val);
	return 0;
}

struct library_tree_node* recursive_search_lib_tree
(
	char** path,
	int lvl,
	struct library_tree_node** crnt
)
{
	if(NULL == crnt)
		return NULL;

	//end of path
	if(NULL == path || NULL == path[lvl])
		return *crnt;

	for(int i=0;NULL != crnt[i];++i)
		if(strcmp(path[lvl],crnt[i]->name))
			return recursive_search_lib_tree(path, lvl+1, crnt[i]->subnodes);

	return NULL;
}

void recursive_search_ahead_lib_tree
(
	const char** path,
	int* lvl,
	struct library_tree_node** crnt,
	struct library_tree_node** last
)
{
	//end of path
	if(NULL == path || NULL == path[*lvl])
		return;

	if(NULL == crnt)
		return;

	for(int i=0;NULL != crnt[i];++i)
		if(0 == strcmp(path[*lvl], crnt[i]->name))
		{
			++(*lvl);
			*last = crnt[i];
			crnt = crnt[i]->subnodes;
			recursive_search_ahead_lib_tree(path, lvl, crnt, last);
			return;
		}
}

void search_ahead_library_path
(
	const char** path,
	int* lvl,
	struct library_tree_node** last
)
{
	if(NULL == ROOT_NODES)
		return;

	int inlvl = 0;
	struct library_tree_node* inlast;

	if(NULL == lvl)
		lvl = &inlvl;

	if(NULL == last)
		last = &inlast;

	recursive_search_ahead_lib_tree(path, lvl, ROOT_NODES, last);
}

struct library_tree_node* get_library_tree_node_by_path(char** path)
{
	return recursive_search_lib_tree(path, 0, ROOT_NODES);
}

struct library_tree_node* get_or_create_library_path(const char** path)
{
	struct library_tree_node* node = NULL;
	int depth = array_pnt_population((void**)path);
	int lvl = 0;
	search_ahead_library_path(path, &lvl, &node);

	if(depth == lvl)
		return node;

	while(NULL != path[lvl])
	{
		struct library_tree_node* new_node = malloc_zero(sizeof(struct library_tree_node));
		new_node->name = path[lvl];
		array_pnt_append_element((void***)(NULL == node? &ROOT_NODES:&node->subnodes), (void*) new_node);
		node = new_node;
		++lvl;
	}

	return node;
}

void dbg_print_leafs(struct library_tree_node* node, int lvl)
{
	if(NULL == node)
		return;

	struct lxc_gate_behavior** ent = node->gates;

	if(NULL == ent)
		return;

	for(int g=0;NULL != ent[g];++g)
	{
		for(int i=0;i<lvl;++i)
			printf("\t");

		printf("`%s`\n", ent[g]->gate_name, ent[g]->gate_name);
	}
}

void dbg_recursive_print_library_tree(struct library_tree_node** node, int lvl, bool leafs)
{
	if(NULL == node)
		return;

	for(int l=0;NULL != node[l];++l)
	{

		for(int i=0;i<lvl;++i)
			printf("\t");

		printf("%s",node[l]->name);
		if(leafs)
		{
			printf(":\n");
			dbg_print_leafs(node[l], lvl+1);
		}
		else
		{
			printf("\n");
		}

		dbg_recursive_print_library_tree(node[l]->subnodes, lvl+1, leafs);
	}
}

void dbg_print_library_tree(bool leafs)
{
	if(NULL == ROOT_NODES)
	{
		printf("No library registered yet.\n");
		return;
	}

	dbg_recursive_print_library_tree(ROOT_NODES, 0, leafs);
}

struct lxc_gate_behavior* get_gate_entry_by_name(const char* name)
{
	if(NULL == REGISTERED_BEHAVIORS)
		return NULL;

	for(int i=0;NULL != REGISTERED_BEHAVIORS[i];++i)
		if(0 == strcmp(name, REGISTERED_BEHAVIORS[i]->gate_name))
			return REGISTERED_BEHAVIORS[i];

	return NULL;
}

int lxc_register_signal(Signal signal)
{
	Signal in = lxc_get_signal_by_name(signal->name);
	if(NULL != in)
		return LXC_ERROR_ENTITY_ALREADY_REGISTERED;

	array_pnt_append_element((void***)&REGISTERED_SIGNALS, (void*) signal);
	return 0;
}


int lxc_register_conversion_function(Signal from, Signal to, LxcValue (*function)(LxcValue))
{
	void* funct = lxc_get_conversion_function(from, to);
	if(NULL != funct)
		return LXC_ERROR_ENTITY_ALREADY_REGISTERED;

	array_pnt_append_element((void*)&(from->cast_to), (void*) function);

	return 0;
}


LxcValue (*lxc_get_conversion_function(Signal from, Signal to))(LxcValue)
{
	if(NULL == from || NULL == to)
		return NULL;

	struct lxc_cast_to** cast = from->cast_to;

	if(NULL == cast)
		return NULL;

	for(int i=0;NULL != cast[i];++i)
		if(to == cast[i]->to)
			return cast[i]->cast_function;

	return NULL;
}

/******************************************************************************/


unsigned int workspace_autoincrement_id = 0;

unsigned int get_next_workspace_id()
{
	return ++workspace_autoincrement_id;
}


//const Workspace BOOTSTRAPPING_WORKSPACE = init_bootstrapping_workspace();


Workspace get_bootstrapping_workspace()
{
	if(NULL == BOOTSTRAPPING_WORKSPACE)
	{
		BOOTSTRAPPING_WORKSPACE = malloc(sizeof(struct workspace));
		memset(BOOTSTRAPPING_WORKSPACE, 0, sizeof(struct workspace));
		BOOTSTRAPPING_WORKSPACE_BUILDER(BOOTSTRAPPING_WORKSPACE);
	}

	return BOOTSTRAPPING_WORKSPACE;
}

int lxc_load_shared_library(const char* so_file, char** errors, int maxlength)
{
	void* handle = dlopen(so_file, RTLD_NOW);
	if(NULL == handle)
	{
		return LXC_ERROR_LIBRARY_SO_CANT_OPEN;
	}

	struct lxc_loadable_library* lib = dlsym(handle, "logxcontroll_loadable_library");
	if(NULL == lib)
	{
		array_fix_try_add_last_null(errors, maxlength, dlerror());
		dlclose(handle);
		return LXC_ERROR_LIBRARY_SYMBOL_NOT_FOUND;
	}
	else
	{
		//TODO on success, register plugin
		return lxc_load_library(lib, errors, maxlength);
	}

	return 0;
};

