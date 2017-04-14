#include "core/logxcontroll.h"

#include "np.h"

struct fake_item
{
	char* fake;
};

static void test_array_pnt(void)
{
	//assets for the null terminated array test

	//yet uninitalized array
	void** array = NULL;

	//4 different element
	struct fake* elem1 = (struct fake*) 0xfade;
	struct fake* elem2 = (struct fake*) 0xdead;
	struct fake* elem3 = (struct fake*) 0xbeef;
	struct fake* elem4 = (struct fake*) 0xface;


	/******************************** init ************************************/

	array_pnt_init(&array);


	//after initialization, the first element is the array terminator NULL.
	NP_ASSERT_EQUAL(array[0], NULL);


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



	/************************* add all other elements *************************/

	NP_ASSERT_EQUAL(1, array_pnt_append_element(&array, elem2));
	NP_ASSERT_EQUAL(2, array_pnt_append_element(&array, elem3));
	NP_ASSERT_EQUAL(3, array_pnt_append_element(&array, elem4));

	/**************************************************************************/

	//can't pop the NULL array terminator
	NP_ASSERT_EQUAL(NULL, array[4]);
	NP_ASSERT_EQUAL(NULL, array_pnt_pop_element(&array, 4));
	NP_ASSERT_EQUAL(NULL, array[4]);

	//if we remove the element at the 0th position, all elements after that
	//must be shifted backward

	NP_ASSERT_EQUAL(elem1, array_pnt_pop_element(&array, 0));

	//not contains the first element anymore
	NP_ASSERT_EQUAL(-1, array_pnt_contains(array, elem1));


	NP_ASSERT_EQUAL(0, array_pnt_contains(array, elem2));
	NP_ASSERT_EQUAL(1, array_pnt_contains(array, elem3));
	NP_ASSERT_EQUAL(2, array_pnt_contains(array, elem4));

	//really at the position, not just sayin'?
	NP_ASSERT_EQUAL(elem2 , array[0]);
	NP_ASSERT_EQUAL(elem3 , array[1]);
	NP_ASSERT_EQUAL(elem4 , array[2]);

	//removing last element
	NP_ASSERT_EQUAL(elem4, array_pnt_pop_element(&array, 2));

	NP_ASSERT_EQUAL(elem2 , array[0]);
	NP_ASSERT_EQUAL(elem3 , array[1]);
	//NP_ASSERT_EQUAL(NULL , array[2]);

	free(array);
}

/*void test_array_fix(void)
{
	//TODO
}*/



