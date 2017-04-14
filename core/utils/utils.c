/*
 * utils.c
 *
 *  Created on: 2016.02.04.
 *      Author: szupervigyor
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "string.h"

#include "core/logxcontroll.h"


void linux_print_heap_size()
{
	char data[2048];
	int fd = open("/proc/self/maps", O_RDONLY);

	read(fd, data, sizeof(data));

	data[sizeof(data)-1] = '0';

	close(fd);

	char* off = strstr(data, "[heap]");

	if(NULL == off)
	{
		printf("no heap initiated\n");
		return;
	}

	while(*off != '\n')
		--off;

	char* sign = strstr(off, "-");

	long low = strtoul(off+1, NULL, 16);

	long high = strtoul(sign+1, NULL, 16);

	long diff = high - low;

	printf("Heap size: %ld Mb, %ld Kb, %ld bytes\n", diff/(1024*1024), diff/1024, diff);
}

#include <execinfo.h>

/**
 *
 * https://www.gnu.org/software/libc/manual/html_node/Backtraces.html
 * */
void gnu_libc_print_stack_trace()
{
	void *array[10];
	size_t size;
	char **strings;
	size_t i;

	size = backtrace(array, 10);
	strings = backtrace_symbols(array, size);

	printf ("Obtained %zd stack frames.\n", size);

	for (i = 0; i < size; i++)
	{
		printf ("%s\n", strings[i]);
	}

	free (strings);
}

char* render_string(int maxlen, char* fmt, void* argptr)
{
	char* ret = (char*) malloc(sizeof(char)*maxlen);

	int len = snprintf(ret, maxlen, fmt, argptr);

	if(len >= maxlen)
	{
		free(ret);
		return NULL;
	}

	ret = realloc(ret, strlen(ret)+1);

	return ret;
}

char* pointer_to_string(void* addr)
{
	COMPILE_TIME_ASSERT(sizeof(void*) <= 12);

	if(NULL == addr)
		return "0x0";

	return render_string(30,"%p", addr);
}

void* string_to_pointer(char* ptr)
{
	COMPILE_TIME_ASSERT(sizeof(void*) <= sizeof(long int));
	return (void*) strtol(ptr, NULL, 16);
}

char* copy_string(const char* str)
{
	char* ret = malloc(strlen(str)+1);
	strcpy(ret, str);
	return ret;
}

void* offset_bytes(void* addr, int bytes)
{
	return ((void*)(((char*)addr) + bytes));
}

void* malloc_zero(size_t size)
{
	void* ret = malloc(size);
	if(NULL != ret)
		memset(ret, 0, size);

	return ret;
}

void* realloc_zero(void* addr, size_t old_len, size_t new_length)
{
	addr = realloc(addr, new_length);

	int diff = new_length - old_len;
	if(diff > 0)
	{
		memset(offset_bytes(addr, old_len), 0, diff);
	}

	return addr;
}

void dbg_print_messages(char** msgs)
{
	if(NULL == msgs)
		return;

	int i;
	for(i=0;NULL != msgs[i];++i)
	{
		printf("%s\n", msgs[i]);
	}
}

void dbg_crash()
{
	int* val = NULL;
	*val = 0;
}

/*
 * pop the specified address from the array and shift the elements to the lower
 * index (stay continuous)
 * */
void* array_nt_pop_element(void** array_addr, unsigned int len, unsigned int index)
{
	if(NULL == array_addr || len <= index)
	{
		return NULL;
	}
	else
	{
		unsigned int i = index;

		void* ret = array_addr[i];

		//shifting the array if least one element on the upper index
		--len;
		//len is now lower, no extra len+1 needed in the next ops.
		while(i < len)
		{
			array_addr[i] = array_addr[i+1];
			++i;
		}

		//set the last element NULL
		//(no duplication on end of the array)
		array_addr[len] = NULL;

		return ret;
	}
}

/**
 * Add element to the and of the array (where NULL found) then terminates the
 * array (if size != length).
 * expands the array if element can't added to.
 * */
void array_nt_append_element(void*** array_addr, unsigned int* length, void* element)
{
	if(NULL == *array_addr)
	{
		*array_addr = malloc(sizeof(void*));
		*length = 1;
		(*array_addr)[0] = element;
	}
	else
	{
		int ep = -1;
		int i=0;
		int len = *length;
		while(i < len)
		{
			if(NULL == (*array_addr)[i])
			{
				ep = i;
				break;
			}
			++i;
		}

		if(-1 == ep)
		//all slot are full (we increase the slot by 1)
		{
			//allocation one more bigger
			*array_addr = realloc(*array_addr, len+sizeof(void*));
			*length = len + 1;
			(*array_addr)[len] = element;
		}
		else
		{
			//set the very next free slot.
			(*array_addr)[ep] = element;
			//and terminate if slot available after
			if(ep+1 < len)
				(*array_addr)[ep+1] = NULL;
		}
	}
}

int array_nt_contains(void** array_addr, unsigned int length, void* addr)
{
	if(NULL == array_addr)
		return -1;

	uint i = 0;
	while(i < length)
	{
		if(NULL == array_addr[i])
			return -1;

		if(addr == array_addr[i])
			return i;

		++i;
	}
	return -1;
}

Wire* find_mixed_wire_location(Wire* mixed_type_wires, unsigned int length, Signal signal, unsigned int index)
{
	if(NULL == mixed_type_wires)
		return NULL;

	unsigned int ordinal = 0;
	uint i = 0;
	for(i=0;i < length; ++i)
	{
		if(NULL != mixed_type_wires[i])
		{
			return NULL;
		}

		if(signal == mixed_type_wires[i]->type)
		{
			if(ordinal == index)
				return &(mixed_type_wires[i]);

			++ordinal;
		}
	}

	return NULL;
}

int array_nt_population(void** array_addr, unsigned int length)
{
	if(NULL == array_addr)
		return 0;

	uint i;
	for(i= 0;i < length;++i)
	{
		if(NULL == array_addr[i])
		{
			return i;
		}
	}

	return length;
}

void array_ensure_index(void*** array_addr, unsigned int* length, unsigned int index)
{
	if(*length > index)
		return;

	*array_addr = realloc(*array_addr, sizeof(void*)*(index+1));
	*length = index+1;
}


/************************* methods for fix arrays *****************************/

void* array_fix_remove_element(void** array_addr, uint length, uint index)
{
	if(NULL == array_addr || length <= index)
		return NULL;

	void* ret = array_addr[index];
	array_addr[index] = NULL;
	return ret;
}

void array_fix_add_element(void*** array_addr, uint* length, void* element)
{
	int index = array_fix_first_free_slot(*array_addr, *length);
	if(-1 == index)
	{
		index = *length;
		//growth the array with 1 slot;
		array_fix_ensure_index(array_addr, length, *length);
	}
	(*array_addr)[index] = element;
}

bool array_fix_try_add_last_null(void** array_addr, uint length, void* element)
{
	int index = array_fix_first_free_slot(array_addr, length);
	if(-1 == index)
	{
		return false;
	}

	array_addr[index] = element;
	return true;
}


void array_fix_ensure_index(void*** array_addr, uint* length, uint index)
{
	if(*length > index)
		return;

	uint oldlen = *length;
	*array_addr = realloc(*array_addr, sizeof(void*)*(index+1));
	memset(&((*array_addr)[oldlen]), 0, (index+1 - oldlen)* sizeof(void*));
	*length = index+1;
}

//returns the index of the first occurrence of item.
//returns -1 if not found
int array_fix_contains(void** array_address, uint length, void* val)
{
	if(NULL == array_address)
		return -1;

	uint i;
	for(i = 0;i<length;++i)
	{
		if(array_address[i] == val)
		{
			return i;
		}
	}

	return -1;
}

int array_fix_population(void** array_address, uint length)
{
	if(NULL == array_address)
		return 0;

	int population = 0;

	uint i;
	for(i = 0;i<length;++i)
	{
		if(NULL != array_address[i])
		{
			++population;
		}
	}

	return population;

}

int array_fix_first_free_slot(void** array_address, uint length)
{
	if(NULL == array_address)
		return -1;

	uint i;
	for(i=0;i<length;++i)
	{
		if(NULL == array_address[i])
		{
			return i;
		}
	}

	return -1;
}


/**************************** packed null terminated **************************/

void array_pnt_init(void*** array_addr)
{
	if(NULL != *array_addr)
		return;

	(*array_addr) = malloc(sizeof(void**));
	(*array_addr)[0] = NULL;
}

int array_pnt_last_index(void** array_addr)
{
	//the array is not yet initalized or the first elemenet is the terminator
	if(NULL == array_addr || NULL == array_addr[0])
	{
		return -1;
	}

	uint i = 1;
	for(;;)
	{
		if(NULL == array_addr[i])
		{
			return i-1;
		}
		++i;
	}
	//here the process will die by segfault (if array corrupted)
	return -1;
}


void* array_pnt_pop_element(void*** array_addr, uint index)
{
	int lst = array_pnt_last_index(*array_addr);
	if(lst < (int) index)
	{
		return NULL;
	}

	void* ret = (*array_addr)[index];

	int i;
	for(i = index;i < lst;++i)//i+1 is NULL at the end of the iteration
	{
		(*array_addr)[i] = (*array_addr)[i+1];
	}

	(*array_addr) = realloc(*array_addr, sizeof(void*)*(lst+2));

	return ret;
}

int array_pnt_append_element(void*** array_addr, void* element)
{
	if(NULL == element)
	{
		return -1;
	}

	int lst = array_pnt_last_index(*array_addr);
	if(lst < 0)
		lst = 0;
	else
		++lst;

	(*array_addr) = realloc(*array_addr, sizeof(void*)*(lst+2));
	(*array_addr)[lst] = element;
	(*array_addr)[lst+1] = NULL;

	return lst;
}

int array_pnt_contains(void** array_addr, void* addr)
{
	if(NULL == array_addr)
		return -1;

	int i;
	for(i=0;NULL != array_addr[i];++i)
	{
		if(addr == array_addr[i])
		{
			return i;
		}
	}

	return -1;
}

int array_pnt_population(void** array_addr)
{
	return array_pnt_last_index(array_addr)+1;
}

void array_pnt_free_all(void** array_addr)
{
	if(NULL == array_addr)
		return;

	int i;
	for(i=0;NULL != array_addr[i];++i)
	{
		free(array_addr[i]);
	}

	free(array_addr);
}

void array_pnt_dbg_printf_char_array(void** array, char* array_name)
{
	if(NULL == array)
	{
		printf("%s array is null\r\n", array_name);
		return;
	}
	else
		printf("%s array address: %p\r\n", array_name, array);

	char** arr = (char**)array;

	int i;
	for(i=0;NULL != arr[i];++i)
	{
		printf("%s[%d] = \"%s\"\r\n", array_name, i, arr[i]);
	}
}

int safe_strcpy(char* dst, int max_length, const char* src)
{
	if(NULL == src || NULL == dst)
		return 0;

	int len = strlen(src);
	if(len < max_length)
	{
		strcpy(dst,src);
	}
	else
	{
		memcpy(dst,src, max_length-1);
		dst[max_length-1] = '\0';
	}

	return len;
}

#include <dlfcn.h>

void gnu_libc_print_backtraced_symbol(void* addr)
{
	char sym[200];
	sym[0] = 0;
	gnu_libc_backtrace_symbol(addr, sym, sizeof(sym));
	printf("%s\n",sym);
}

int gnu_libc_backtrace_symbol(void* addr, char* ret_str, size_t max_length)
{
	void *arr[1];
	arr[0] = addr;
	char** ob = backtrace_symbols(arr, 1);
	if(NULL == ob)
	{
		return safe_strcpy(ret_str,max_length, "");
	}

	char* ret = ob[0];

	if(NULL != ret)
	{
		int retval = safe_strcpy(ret_str,max_length, ret);
		free(ob);
		return retval;
	}
	else
	{
		return safe_strcpy(ret_str,max_length, "");
	}

	/*struct Dl_info info;
	if(0 == dladdr(addr, &info))
	{
		return safe_strcpy(ret_str, max_length, "");
	}

	return safe_strcpy(ret_str, max_length, info.dli_sname);
	*/
}

void queue_add_element
(
	struct queue_element** head,
	struct queue_element* elem,
	struct queue_element** tail
)
{
	if(NULL == *head)
	{
		*head = elem;
		*tail = elem;
		elem->next = NULL;
		elem->prev = NULL;
	}
	else
	{
		(*tail)->next = elem;

		elem->next = NULL;
		elem->prev = *tail;

		*tail = elem;
	}
}

struct queue_element* queue_pop_tail_element
(
	struct queue_element** head,
	struct queue_element** tail
)
{
	if(NULL == *tail)
	{
		return NULL;
	}
	else
	{
		struct queue_element* ret = *tail;

		if(NULL != ret->prev)
		{
			//set the tail with the previous one
			*tail = ret->prev;

			//unbind returning element
			ret->prev->next = NULL;

			//unbind reference to the queue
			ret->prev = NULL;
		}
		else
		//this means only this element in the queue
		{
			*head = NULL;
			*tail = NULL;
			//and ret->next nad ret->prev in this case is NULL
		}

		//ret->next is always NULL!
		return ret;
	}
}

void queue_pop_intermediate_element
(
	struct queue_element** head,
	struct queue_element* intermediate,
	struct queue_element** tail
)
{
	struct queue_element* op;

	//first element of the queue
	if(NULL == intermediate->prev)
	{
		*head = intermediate->next;
		op = intermediate->next;

		if(NULL == op)
		//this is the only element, empty the queue
		{
			*tail = NULL;
		}
		else
		{
			op->prev = NULL;
		}
	}
	else
	//set the previous element's next
	{
		intermediate->prev = intermediate->next;
	}

	//last element of the queue
	if(NULL == intermediate->next)
	{
		*tail = intermediate->prev;
		op = intermediate->prev;

		if(NULL == op)
		//this is the only element
		{
			*head = NULL;
		}
		else
		{
			op->next = NULL;
		}
	}
	else
	//set the next element's prev
	{
		intermediate->next = intermediate->prev;
	}
}

void print_checkpoint(char* str)
{
	printf("%s\r\n", str);
	fsync(1);
}
