
#include "core/logxcontroll.h"

#include <pcre.h>

#define REGEX_GROUP_MAX_LENGTH 20

struct compiled_regex
{
	pcre* regex;
	pcre_extra* opt_regex;
};

typedef struct compiled_regex* Regex;

struct regex_matcher
{
	Regex regex;
	const char* str;
	int group_count;
	int res_vec[REGEX_GROUP_MAX_LENGTH];
};

typedef struct regex_matcher* RegexMatcher;


int regex_destroy(Regex regex);
int regex_compile(Regex regex, const char* expr, int options, const char** error);

int regex_get_group(RegexMatcher match, int grp, const char** dst);
void regex_free_group(const char* dst);
