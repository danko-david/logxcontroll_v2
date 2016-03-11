/*
 * playground.c
 *
 *  Created on: 2016.01.22.
 *      Author: szupervigyor
 */

#include <stdio.h>
#include <unistd.h>
/*
struct ancestor
{
	char mychar;
	int myint;
	void* myptr;
};

struct descendant
{
	struct ancestor top;
	char mychar;
	int myint;
};

void init(struct ancestor* anc)
{
	anc->mychar = 'a';
	anc->myint = 12;
	anc->myptr = NULL;
}*/
/*
	//printf("%s\n", argv[0]);
	//sync();

	struct descendant* subject = (struct descendant*) malloc(sizeof( struct descendant));
	subject->mychar = 'b';
	subject->myint = 24;

	printf("desc: %c %d\n", subject->mychar, subject->myint);

	init((struct ancestor*) subject);

	printf("anc: %c %d %p\n", subject->top.mychar, subject->top.myint, subject->top.myptr);

	printf("desc: %c %d\n", subject->mychar, subject->myint);

	return 0;
*/
/*
struct base_data
{
	const char* opts;
	int num;
	void (*handle_special_operation)(void* param);
};


struct specific_data
{
	struct base_data base_data;
	const char* spec_opts;
};

void spec_print_data(void* data)
{
	struct specific_data* spec = data;
	printf("special operation: %s\n", spec->spec_opts);
}

struct specific_data* create_specific_data(const char* opts, const char* spec_opts)
{
	struct specific_data* ret = malloc(sizeof ret);
	ret->base_data.opts = opts;
	ret->base_data.num = 12;
	ret->base_data.handle_special_operation = spec_print_data;
	ret->spec_opts = spec_opts;
	return ret;
}

void do_generic_operation(struct base_data* data)
{
	printf("base_opts: %s\n", data->opts);
}

void do_specific_operion(struct base_data* data)
{
	data->handle_special_operation(data);
}

int alterantive_main()
{
	struct specific_data* data = create_specific_data("base options", "i'm different");
	struct base_data* datas[5];
	datas[0] = data;
	//type info lost

	do_generic_operation(data);
	do_specific_operion(data);

	return 0;
}
*/
