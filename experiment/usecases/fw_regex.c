
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

			//every acquired group should be released (in the background)
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


void fw_regex(int argc, char **argv, int start_from)
{
	test_regex_find_float_price();
	test_regex_find_float_price_named();
}
