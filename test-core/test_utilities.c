
#include "test-core/test_core.h"

struct fake_item
{
	char* fake;
};

static struct fake* elem1 = (struct fake*) 0xfade;
static struct fake* elem2 = (struct fake*) 0xdead;
static struct fake* elem3 = (struct fake*) 0xbeef;
static struct fake* elem4 = (struct fake*) 0xface;

static void create_array_pnt_with_4_element(void*** array)
{
	array_pnt_init(array);
	NP_ASSERT_EQUAL(0, array_pnt_append_element(array, elem1));
	NP_ASSERT_EQUAL(1, array_pnt_append_element(array, elem2));
	NP_ASSERT_EQUAL(2, array_pnt_append_element(array, elem3));
	NP_ASSERT_EQUAL(3, array_pnt_append_element(array, elem4));

}

static void test_array_pnt_with_4_element(void)
{
	void** array = NULL;
	create_array_pnt_with_4_element(&array);

	/**************************************************************************/

	//can't pop the NULL array terminator
	NP_ASSERT_PTR_EQUAL(NULL, array[4]);
	NP_ASSERT_PTR_EQUAL(NULL, array_pnt_pop_element(&array, 4));
	NP_ASSERT_PTR_EQUAL(NULL, array[4]);

	//if we remove the element at the 0th position, all elements after that
	//must be shifted backward

	NP_ASSERT_PTR_EQUAL(elem1, array_pnt_pop_element(&array, 0));

	//not contains the first element anymore
	NP_ASSERT_EQUAL(-1, array_pnt_contains(array, elem1));


	NP_ASSERT_EQUAL(0, array_pnt_contains(array, elem2));
	NP_ASSERT_EQUAL(1, array_pnt_contains(array, elem3));
	NP_ASSERT_EQUAL(2, array_pnt_contains(array, elem4));

	//really at the position, not just sayin'?
	NP_ASSERT_PTR_EQUAL(elem2 , array[0]);
	NP_ASSERT_PTR_EQUAL(elem3 , array[1]);
	NP_ASSERT_PTR_EQUAL(elem4 , array[2]);

	//removing last element
	NP_ASSERT_PTR_EQUAL(elem4, array_pnt_pop_element(&array, 2));

	NP_ASSERT_PTR_EQUAL(elem2 , array[0]);
	NP_ASSERT_PTR_EQUAL(elem3 , array[1]);
	//NP_ASSERT_EQUAL(NULL , array[2]);
	free(array);
}

static void test_array_pnt_first_element_added(void)
{
	//yet uninitalized array
	void** array = NULL;

	/******************************** init ************************************/

	array_pnt_init(&array);


	//after initialization, the first element is the array terminator NULL.
	NP_ASSERT_PTR_EQUAL(array[0], NULL);


	//and the size is zero
	NP_ASSERT_EQUAL(0, array_pnt_population(array));

	/************************* adding the first element ***********************/

	//added to the 0-th position
	NP_ASSERT_EQUAL(0, array_pnt_append_element(&array, elem1));


	//size is 1
	NP_ASSERT_EQUAL(1, array_pnt_population(array));


	//contains the element at the 0-nth position
	NP_ASSERT_EQUAL(0, array_pnt_contains(array, elem1));


	//adding NULL to the array is an illegal operation,
	//and returns -1 as position
	NP_ASSERT_EQUAL(-1, array_pnt_append_element(&array, NULL));

	free(array);
}

static void test_array_fix(void)
{
	void** array = NULL;
	uint len = 0;

	{
		NP_ASSERT_EQUAL(0, array_fix_population(array, len));
	}

	{
		array_fix_add_element(&array, &len, elem1);

		NP_ASSERT_PTR_EQUAL(elem1, array[0]);

		NP_ASSERT_EQUAL(1, array_fix_population(array, len));
		NP_ASSERT_EQUAL(1, len);
	}

	{
		//should have no effect, but may increases the length
		array_fix_add_element(&array, &len, NULL);
		NP_ASSERT_PTR_EQUAL(elem1, array[0]);
		NP_ASSERT_EQUAL(1, array_fix_population(array, len));

		//adding NULL may increase the length of the array
		//if there's no intermediate element
		////////NP_ASSERT_EQUAL(1, len);
	}

	{
		array_fix_add_element(&array, &len, elem2);
		array_fix_add_element(&array, &len, elem3);
		array_fix_add_element(&array, &len, elem4);

		//really at the position, not just sayin'?
		NP_ASSERT_PTR_EQUAL(elem1, array[0]);
		NP_ASSERT_PTR_EQUAL(elem2, array[1]);
		NP_ASSERT_PTR_EQUAL(elem3, array[2]);
		NP_ASSERT_PTR_EQUAL(elem4, array[3]);


	}

	{
		//with other words
		NP_ASSERT_EQUAL(0, array_fix_contains(array, len, elem1));
		NP_ASSERT_EQUAL(1, array_fix_contains(array, len, elem2));
		NP_ASSERT_EQUAL(2, array_fix_contains(array, len, elem3));
		NP_ASSERT_EQUAL(3, array_fix_contains(array, len, elem4));
	}


	NP_ASSERT_PTR_EQUAL(elem1, array_fix_remove_element(array, len, 0));

	{
		//deleting just set the slot to NULL
		NP_ASSERT_PTR_EQUAL(NULL, array[0]);
		NP_ASSERT_PTR_EQUAL(elem2, array[1]);
		NP_ASSERT_PTR_EQUAL(elem3, array[2]);
		NP_ASSERT_PTR_EQUAL(elem4, array[3]);
	}


	free(array);
}

static void test_array_pt_with_4_element(void)
{
	void** array = NULL;
	int len = 0;

	array_nt_init(&array, &len);

	NP_ASSERT_EQUAL(0, array_nt_append_element(&array, &len, elem1));
	NP_ASSERT_EQUAL(1, array_nt_append_element(&array, &len, elem2));
	NP_ASSERT_EQUAL(2, array_nt_append_element(&array, &len, elem3));
	NP_ASSERT_EQUAL(3, array_nt_append_element(&array, &len, elem4));

	/**************************************************************************/

	//can't pop the NULL array terminator
	NP_ASSERT_PTR_EQUAL(NULL, array_nt_pop_element(array, len, 4));

	//if we remove the element at the 0th position, all elements after that
	//must be shifted backward

	NP_ASSERT_PTR_EQUAL(elem1, array_nt_pop_element(array, len, 0));

	//not contains the first element anymore
	NP_ASSERT_EQUAL(-1, array_nt_contains(array, len, elem1));


	NP_ASSERT_EQUAL(0, array_nt_contains(array, len, elem2));
	NP_ASSERT_EQUAL(1, array_nt_contains(array, len, elem3));
	NP_ASSERT_EQUAL(2, array_nt_contains(array, len, elem4));

	//really at the position, not just sayin'?
	NP_ASSERT_PTR_EQUAL(elem2 , array[0]);
	NP_ASSERT_PTR_EQUAL(elem3 , array[1]);
	NP_ASSERT_PTR_EQUAL(elem4 , array[2]);

	//removing last element
	NP_ASSERT_PTR_EQUAL(elem4, array_nt_pop_element(array, len, 2));

	NP_ASSERT_PTR_EQUAL(elem2 , array[0]);
	NP_ASSERT_PTR_EQUAL(elem3 , array[1]);
	//NP_ASSERT_EQUAL(NULL , array[2]);
	free(array);
}

static void test_array_nt_first_element_added(void)
{
	//yet uninitalized array
	void** array = NULL;
	int len = 0;
	/******************************** init ************************************/
	array_nt_init(&array, &len);

	//after initialization, the first element is the array terminator NULL.
	NP_ASSERT_PTR_EQUAL(array[0], NULL);


	//and the size is zero
	NP_ASSERT_EQUAL(0, array_nt_population(array, len));

	/************************* adding the first element ***********************/

	//added to the 0-th position
	NP_ASSERT_EQUAL(0, array_nt_append_element(&array, &len, elem1));


	//size is 1
	NP_ASSERT_EQUAL(1, array_nt_population(array, len));


	//contains the element at the 0-nth position
	NP_ASSERT_EQUAL(0, array_nt_contains(array, len, elem1));


	//adding NULL to the array is an illegal operation,
	//and returns -1 as position
	NP_ASSERT_EQUAL(-1, array_nt_append_element(&array, &len, NULL));

	free(array);
}
