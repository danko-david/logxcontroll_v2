/*
 * acconting.h
 *
 *  Created on: 2016.02.15.
 *      Author: szupervigyor
 */

#include "core/logxcontroll.h"

#ifndef ACCONTING_H_
#define ACCONTING_H_

/*
 	 dl should looking for "logxcontroll_loadable_library_" prefix symbol,
 	 witch a `struct loadable_library`

 * */

/*
 * Builtin fuction used for search signals. It's also essential for library
 * */
extern Signal* REGISTERED_SIGNALS;
extern struct lxc_constant_value** REGISTERED_CONSTANT_VALUES;

int lxc_register_signal(Signal);

extern struct lxc_gate_behavior** REGISTERED_BEHAVIORS;
//TODO constant pool

struct library_tree_node* get_or_create_library_path(const char** path);

int lxc_load_library(const struct lxc_loadable_library* lib, const char** error, int max_length);
int lxc_register_gate(struct lxc_gate_behavior* entry);
struct lxc_gate_behavior* get_gate_entry_by_name(const char* name);




LxcValue (*lxc_get_conversion_function(Signal from, Signal to))(LxcValue);
int lxc_register_conversion_function(Signal from, Signal to, LxcValue (*function)(LxcValue));

int lxc_register_constant_value(struct lxc_constant_value* val);

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


int lxc_load_shared_library(const char* so_file, const char** error, int maxlength);


/***************************/

struct library_tree_node
{
	const char* name;

	//array_pnt of gate behaviors;
	struct lxc_gate_behavior** gates;

	//array_pnt of more node;
	struct library_tree_node** subnodes;
};

extern struct library_tree_node** ROOT_NODES;


/******************************* WORKSPACE DEFS *******************************/

typedef struct lxc_workspace* Workspace;

//extern unsigned int workspace_autoincrement_id = 0;

unsigned int get_next_workspace_id();

struct lxc_workspace
{
	struct circuit circuit;

	unsigned int id;
	char* name;

	//wire extra info
	//gate extra info

	//execution pool
};

extern Workspace* WORKSPACES;



/************************ BOOTSTRAPPING WORKSPACE *****************************/
Workspace BOOTSTRAPPING_WORKSPACE;

Workspace get_bootstrapping_workspace();

void default_bootstrapping_workspace_builder(Workspace);

#ifndef BOOTSTRAPPING_WORKSPACE_BUILDER
	#define BOOTSTRAPPING_WORKSPACE_BUILDER default_bootstrapping_workspace_builder;
#endif

/************************* LIBRARY REGISTRATION *******************************/


#endif /* ACCONTING_H_ */
