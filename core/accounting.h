/*
 * acconting.h
 *
 *  Created on: 2016.02.15.
 *      Author: szupervigyor
 */

#include "core/logxcontroll.h"

#ifndef ACCONTING_H_
#define ACCONTING_H_

#include <dlfcn.h>


/*
 	 dl should looking for "logxcontroll_loadable_library_" prefix symbol,
 	 witch a `struct loadable_library`

 * */

/*
 * Builtin fuction used for search signals. It's also essential for library
 * */
extern Signal* REGISTERED_SIGNALS;

int lxc_register_signal(Signal);
Signal lxc_get_signal_by_name(const char*);

extern struct detailed_gate_entry** REGISTERED_BEHAVIORS;
//TODO constant pool
LxcValue lxc_get_constant_by_name(char* name);
struct library_tree_node* get_or_create_library_path(const char** path);

int lxc_load_library(const struct loadable_library* lib, char* error, int max_length);
int lxc_register_gate(struct detailed_gate_entry* entry);
struct detailed_gate_entry* get_gate_entry_by_name(const char* name);

Gate lxc_new_instance_by_name(const char* name);

void dbg_print_library_tree(bool leafs);
/*
 *
 *
 * like: boolen true, false
 * useful constants like: AF_INET as int
 *
 */

/*
 * Basically for loadable so
 */
struct lxc_lib
{
	void* dl_handle;
	struct loadable_library* library;
};


int lxc_load_shared_library(const char* so_file, char* error, int maxlength);


/***************************/

struct library_tree_node
{
	const char* name;

	//array_pnt of gate behaviors;
	struct detailed_gate_entry** gates;

	//array_pnt of more node;
	struct library_tree_node** subnodes;
};

extern struct library_tree_node** ROOT_NODES;


/******************************* WORKSPACE DEFS *******************************/

typedef struct workspace* Workspace;

//extern unsigned int workspace_autoincrement_id = 0;

unsigned int get_next_workspace_id();

struct workspace
{
	struct circuit circuit;

	unsigned int id;
	char* name;

	//wire extra info
	//gate extra info

	//execution pool
};

/************************ BOOTSTRAPPING WORKSPACE *****************************/
Workspace BOOTSTRAPPING_WORKSPACE;

Workspace get_bootstrapping_workspace();

void default_bootstrapping_workspace_builder(Workspace);

#ifndef BOOTSTRAPPING_WORKSPACE_BUILDER
	#define BOOTSTRAPPING_WORKSPACE_BUILDER default_bootstrapping_workspace_builder;
#endif

/************************* LIBRARY REGISTRATION *******************************/


#endif /* ACCONTING_H_ */
