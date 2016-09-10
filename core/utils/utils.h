/*
 * utils.h
 *
 *  Created on: 2016.02.04.
 *      Author: szupervigyor
 */

#ifndef UTILS_H_
#define UTILS_H_

#define STATIC_ASSERT(COND,MSG) typedef char static_assertion_##MSG[(!!(COND))*2-1]

// token pasting madness:
#define COMPILE_TIME_ASSERT3(X,L) STATIC_ASSERT(X,static_assertion_at_line_##L)
#define COMPILE_TIME_ASSERT2(X,L) COMPILE_TIME_ASSERT3(X,L)
#define COMPILE_TIME_ASSERT(X)    COMPILE_TIME_ASSERT2(X,__LINE__)

#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif

void linux_print_heap_size();

/**
 * to fully utilize this function use -rdynamic linking option
 * */
void gnu_libc_print_stack_trace();

/*
	Function for manage arrays used in lxc.
	- Generally we initialize an empty structure
		with 0 size and NULL array pointer.

	- Arrays are continuous and terminated with NULL
		if elements are less than array size.
*/

char* pointer_to_string(void*);

void* string_to_pointer(char*);

char* render_string(int maxlen, char* fmt, void* argptr);

char* copy_string(const char* str);

void* offset_bytes(void* addr, int bytes);

void* malloc_zero(size_t);

void* realloc_zero(void* addr, size_t old_len, size_t new_length);

void dbg_print_messages(char** msgs);

void dbg_crash();

/**
 * Array types:
 * 	- null terminated arrays.
 * 		array_nt_*
 *
 *	- fix
 *		array_fix_*
 *
 *
 *
 * */



/*
 * pop the specified address from the array and shift the elements to the lower
 * index (stay continuous)
 * */
void* array_nt_pop_element(void** array_addr, uint length, uint index);

/**
 * Add element to the and of the array (where NULL found) then terminates the
 * array (if size != length).
 * expands the array if element can't added to.
 * */
void array_nt_append_element(void*** array_addr, uint* length, void* element);

int array_nt_contains(void** array_addr, uint length, void* addr);

int array_nt_population(void** array_addr, uint length);


//void array_nt_ensure_index(void*** array_addr, unsigned int* length, unsigned int index);

void* array_fix_remove_element(void** array_addr, uint length, uint index);

//add element into the first free slot
void array_fix_add_element(void*** array_addr, uint* length, void* element);

bool array_fix_try_add_last_null(void** array_addr, uint length, void* element);

void array_fix_set_element(void*** array_addr, uint* length, uint index, void* element);

void array_fix_ensure_index(void*** array_addr, uint* length, uint index);

int array_fix_contains(void** array_address, uint length, void* val);

int array_fix_population(void** array_address, uint length);

int array_fix_first_free_slot(void** array_address, uint length);

/**
 * packed null terminated array
 * an array where NULL not permitted as intermediate value,
 * only place where must occurred is the end of the array.
 * */
void array_pnt_init(void*** array_addr);

int array_pnt_last_index(void** array_addr);

void* array_pnt_pop_element(void*** array_addr, uint index);

int array_pnt_append_element(void*** array_addr, void* element);

int array_pnt_contains(void** array_addr, void* addr);

int array_pnt_population(void** array_addr);

void array_pnt_free_all(void** array_addr);




void array_pnt_dbg_printf_char_array(void** array, char* array_name);


int safe_strcpy(char* dst, int max_length, const char* src);

int gnu_libc_backtrace_symbol(void* addr, char* ret_str, size_t max_length);

void gnu_libc_print_backtraced_symbol(void* addr);

Wire* find_mixed_wire_location(Wire* mixed_type_wires, uint length, Signal signal, uint index);

struct key_value
{
	void* key;
	void* value;
};

//TODO rename DobleLinkedList

struct queue_element
{
	struct queue_element* prev;
	struct queue_element* next;
};

void queue_add_element
(
	struct queue_element** head,
	struct queue_element* elem,
	struct queue_element** tail
);

struct queue_element* queue_pop_tail_element
(
	struct queue_element** head,
	struct queue_element** tail
);

void queue_pop_intermediate_element
(
	struct queue_element** head,
	struct queue_element* intermediate,
	struct queue_element** tail
);

//TODO add map element
//TODO remove map element
//TODO map key_val by key

void print_checkpoint(char* str);


#endif /* UTILS_H_ */
