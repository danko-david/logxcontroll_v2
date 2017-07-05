
#include "core/logxcontroll.h"

void test_regex_find_float_price()
{
	struct compiled_regex regex;
	const char* err;
	int ret = regex_compile(&regex, "\\s*((\\d\\s*)+([,.]\\d+)?)\\s*(\\w+)", 0, &err);
	if(0 != ret)
	{
		printf("regex compile error: %s\n", err);
		exit(1);
	}

	struct regex_matcher matcher;

	ret = regex_match(&matcher, &regex, "126 143,15 Ft");

	if(0 != ret)
	{
		#ifdef NP_ASSERT
			NP_ASSERT(0 != ret);
		#endif
		printf("Pattern not found\n");
		exit(2);
	}

	const char* str;
	ret = regex_get_group(&matcher, 0, &str);
	if(0 != ret)
	{
		#ifdef NP_ASSERT
			NP_ASSERT(0 != ret);
		#endif
		printf("Failed to get full group\n");
		exit(3);
	}

	printf("Full group: %s\n", str);

	#ifdef NP_ASSERT
		NP_ASSERT_EQUAL(0, strcmp(str, "126 143,15 Ft"));
	#endif

	regex_free_group(str);

	{
		#ifdef NP_ASSERT
			NP_ASSERT_EQUAL(5, matcher.group_count);
		#endif

		int i = 0;
		while(++i < matcher.group_count)
		{
			ret = regex_get_group(&matcher, i, &str);
			if(0 != ret)
			{
				printf("Failed to get %d. group\n", i);
				exit(4);
			}
			printf("group %d: %s\n", i, str);

			#ifdef NP_ASSERT
				if(4 == i)
				{
					NP_ASSERT_EQUAL(0, strcmp("Ft", str));
				}
			#endif

			//every acquired group should be released
			regex_free_group(str);
		}
	}

	regex_destroy(&regex);
}

void test_regex_find_float_price_named()
{
	struct compiled_regex regex;
	const char* err;
	int ret = regex_compile
	(
		&regex,
		"\\s*(?P<price>(\\d\\s*)+([,.]\\d+)?)\\s*(?P<currency>\\w+)",
		0,
		&err
	);

	if(0 != ret)
	{
		printf("regex compile error: %s\n", err);
		exit(1);
	}

	struct regex_matcher matcher;

	ret = regex_match(&matcher, &regex, "126 143,15 Ft");

	if(0 != ret)
	{
		#ifdef NP_ASSERT
			NP_ASSERT(0 != ret);
		#endif
		printf("Pattern not found\n");
		exit(2);
	}

	const char* str;
	ret = regex_get_group(&matcher, 0, &str);
	if(0 != ret)
	{
		#ifdef NP_ASSERT
			NP_ASSERT(0 != ret);
		#endif
		printf("Failed to get full group\n");
		exit(3);
	}

	printf("Full group: %s\n", str);

	#ifdef NP_ASSERT
		NP_ASSERT_EQUAL(0, strcmp(str, "126 143,15 Ft"));
	#endif

	regex_free_group(str);

	const char* grps[] =
	{
		"price",
		"currency",
		NULL
	};


	{
		int i = -1;
		while(NULL != grps[++i])
		{
			ret = regex_get_named_group(&matcher, grps[i], &str);
			if(0 != ret)
			{
				printf("Failed to get \"%s\" group\n", grps[i]);
				exit(4);
			}

			printf("Group \"%s\": %s\n", grps[i], str);

			regex_free_group(str);
		}
	}

	regex_destroy(&regex);
}

void test_regex_overgroup_slice_pass()
{
	struct compiled_regex regex;
	const char* err;
	int ret = regex_compile(&regex, "(\\d)+", 0, &err);
	if(0 != ret)
	{
		printf("regex compile error: %s\n", err);
		exit(1);
	}

	struct regex_matcher matcher;


	char* subject = NULL;

	{
		int size = sizeof(char) * 2 * REGEX_GROUP_MAX_LENGTH;
		subject = malloc(size+1);
		subject[size] = 0;
		int i = -1;
		while(++i != size)
		{
			subject[i] = '0' + (i % 10);
		}
	}

	ret = regex_match(&matcher, &regex, subject);

	if(0 != ret)
	{
		#ifdef NP_ASSERT
			NP_ASSERT(0 != ret);
		#endif
		printf("Pattern not found\n");
		exit(2);
	}

	const char* str;
	ret = regex_get_group(&matcher, 0, &str);
	if(0 != ret)
	{
		#ifdef NP_ASSERT
			NP_ASSERT(0 != ret);
		#endif
		printf("Failed to get full group\n");
		exit(3);
	}

	printf("Full group: %s\n", str);

	regex_free_group(str);

	#ifdef NP_ASSERT
		NP_ASSERT_EQUAL(2, matcher.group_count);
	#endif

	{
		int i = 0;
		while(++i < matcher.group_count)
		{
			ret = regex_get_group(&matcher, i, &str);
			if(0 != ret)
			{
				printf("Failed to get %d. group\n", i);
				exit(4);
			}
			printf("group %d: %s\n", i, str);

			//every acquired group should be released
			//(in the background it's an allocated block)
			regex_free_group(str);
		}
	}

	free(subject);
	regex_destroy(&regex);
}


void fw_regex(int argc, char **argv, int start_from)
{
	//test_regex_find_float_price();
	//test_regex_find_float_price_named();
	test_regex_overgroup_slice_pass();
}
