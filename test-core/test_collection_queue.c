/*
 * test_collection_queue.c
 *
 *  Created on: 2017.06.03.
 *      Author: szupervigyor
 */
#include "test-core/test_core.h"


static const QE_SIZE = sizeof(struct queue_element);
static struct queue_element* queue_element_create_new(int size)
{
	TEST_ASSERT_TRUE(size >= sizeof(struct queue_element));
	struct queue_element* ret = malloc_zero(size);
	return ret;
}

static void queue_free_all(struct queue_element** HEAD,struct queue_element** TAIL)
{
	while(NULL != *TAIL)
	{
		struct queue_element* in = *TAIL;
		queue_pop_intermediate_element(HEAD, *TAIL, TAIL);
		free(in);
	}
}

static void test_queue__one_element_add__head_tail_match(void)
{
	struct queue_element* HEAD = NULL;
	struct queue_element* TAIL = NULL;

	struct queue_element* first = queue_element_create_new(QE_SIZE+10);
	queue_add_element(&HEAD, first, &TAIL);

	TEST_ASSERT_PTR_EQUAL(HEAD, first);
	TEST_ASSERT_PTR_EQUAL(TAIL, first);

	queue_free_all(&HEAD, &TAIL);
}

static void assert_queue_3_element_right_references
(
	struct queue_element* first,
	struct queue_element* secound,
	struct queue_element* third
)
{
	TEST_ASSERT_PTR_EQUAL(first->prev, NULL);
	TEST_ASSERT_PTR_EQUAL(first->next, secound);

	TEST_ASSERT_PTR_EQUAL(secound->prev, first);
	TEST_ASSERT_PTR_EQUAL(secound->next, third);

	TEST_ASSERT_PTR_EQUAL(third->prev, secound);
	TEST_ASSERT_PTR_EQUAL(third->next, NULL);
}

static void test_queue__3_element__right_references(void)
{
	struct queue_element* HEAD = NULL;
	struct queue_element* TAIL = NULL;

	struct queue_element* first = queue_element_create_new(QE_SIZE+10);
	struct queue_element* secound = queue_element_create_new(QE_SIZE+20);
	struct queue_element* third = queue_element_create_new(QE_SIZE+30);
	queue_add_element(&HEAD, first, &TAIL);
	queue_add_element(&HEAD, secound, &TAIL);
	queue_add_element(&HEAD, third, &TAIL);

	TEST_ASSERT_PTR_EQUAL(HEAD, first);

	assert_queue_3_element_right_references(first, secound, third);

	TEST_ASSERT_PTR_EQUAL(TAIL, third);

	queue_free_all(&HEAD, &TAIL);
}

static void test_queue__3_element_first_removed__right_references(void)
{
	struct queue_element* HEAD = NULL;
	struct queue_element* TAIL = NULL;

	struct queue_element* first = queue_element_create_new(QE_SIZE+10);
	struct queue_element* secound = queue_element_create_new(QE_SIZE+20);
	struct queue_element* third = queue_element_create_new(QE_SIZE+30);
	queue_add_element(&HEAD, first, &TAIL);
	queue_add_element(&HEAD, secound, &TAIL);
	queue_add_element(&HEAD, third, &TAIL);

	TEST_ASSERT_PTR_EQUAL(HEAD, first);

	assert_queue_3_element_right_references(first, secound, third);

	TEST_ASSERT_PTR_EQUAL(TAIL, third);

	//do the pop
	{
		queue_pop_intermediate_element(&HEAD, secound, &TAIL);

		free(secound);

		TEST_ASSERT_PTR_EQUAL(HEAD, first);

		TEST_ASSERT_PTR_EQUAL(first->prev, NULL);
		TEST_ASSERT_PTR_EQUAL(first->next, third);

		TEST_ASSERT_PTR_EQUAL(third->prev, first);
		TEST_ASSERT_PTR_EQUAL(third->next, NULL);

		TEST_ASSERT_PTR_EQUAL(TAIL, third);
	}

	queue_free_all(&HEAD, &TAIL);
}

static void test_queue__3_element_head_removed__right_references(void)
{
	struct queue_element* HEAD = NULL;
	struct queue_element* TAIL = NULL;

	struct queue_element* first = queue_element_create_new(QE_SIZE+10);
	struct queue_element* secound = queue_element_create_new(QE_SIZE+20);
	struct queue_element* third = queue_element_create_new(QE_SIZE+30);
	queue_add_element(&HEAD, first, &TAIL);
	queue_add_element(&HEAD, secound, &TAIL);
	queue_add_element(&HEAD, third, &TAIL);

	TEST_ASSERT_PTR_EQUAL(HEAD, first);

	assert_queue_3_element_right_references(first, secound, third);

	TEST_ASSERT_PTR_EQUAL(TAIL, third);

	//do the pop
	{
		queue_pop_intermediate_element(&HEAD, first, &TAIL);

		free(first);

		TEST_ASSERT_PTR_EQUAL(HEAD, secound);

		TEST_ASSERT_PTR_EQUAL(secound->prev, NULL);
		TEST_ASSERT_PTR_EQUAL(secound->next, third);

		TEST_ASSERT_PTR_EQUAL(third->prev, secound);
		TEST_ASSERT_PTR_EQUAL(third->next, NULL);

		TEST_ASSERT_PTR_EQUAL(TAIL, third);
	}

	queue_free_all(&HEAD, &TAIL);
}


static void test_queue__3_element_tail_removed__right_references(void)
{
	struct queue_element* HEAD = NULL;
	struct queue_element* TAIL = NULL;

	struct queue_element* first = queue_element_create_new(QE_SIZE+10);
	struct queue_element* secound = queue_element_create_new(QE_SIZE+20);
	struct queue_element* third = queue_element_create_new(QE_SIZE+30);
	queue_add_element(&HEAD, first, &TAIL);
	queue_add_element(&HEAD, secound, &TAIL);
	queue_add_element(&HEAD, third, &TAIL);

	TEST_ASSERT_PTR_EQUAL(HEAD, first);

	assert_queue_3_element_right_references(first, secound, third);

	TEST_ASSERT_PTR_EQUAL(TAIL, third);

	//do the pop
	{
		queue_pop_intermediate_element(&HEAD, third, &TAIL);

		free(third);

		TEST_ASSERT_PTR_EQUAL(HEAD, first);

		TEST_ASSERT_PTR_EQUAL(first->prev, NULL);
		TEST_ASSERT_PTR_EQUAL(first->next, secound);

		TEST_ASSERT_PTR_EQUAL(secound->prev, first);
		TEST_ASSERT_PTR_EQUAL(secound->next, NULL);

		TEST_ASSERT_PTR_EQUAL(TAIL, secound);
	}

	queue_free_all(&HEAD, &TAIL);
}
