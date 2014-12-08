/* (C) Copyright IBM Corp. 2007, 2008
         Author: Ian Craggs
*/

#include "Topics.h"
#include "Messages.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "Log.h"

int Topics_isValidName(char* aName)
{
	int rc = true;
	char *c = NULL;
	int length = strlen(aName);

	// '#' wildcard can be only at the beginning or the end of a topic
	char* hashpos = strchr(aName, '#');
	if (hashpos != NULL)
	{
		char* second = strchr(hashpos+1, '#');
		if ((hashpos != aName && hashpos != aName+(length-1)) || second != NULL)
			rc = false;
	}

	/* '#' or '+' only next to a slash separator or end of name */
	for (c = "#+"; *c != '\0'; ++c)
	{
		char* pos = strchr(aName, *c);
		while (pos != NULL)
		{
			if (pos > aName)                  /* check previous char is '/'*/
			{
				if (*(pos - 1) != '/')
					rc = false;
			}
			if (*(pos + 1) != '\0')      /* check that subsequent char is '/'*/
			{
				if (*(pos + 1) != '/')
					rc = false;
			}
			pos = strchr(pos + 1, *c);
		}
	}
	return rc;
}


#if defined(WIN32)
#define strtok_r strtok_s
#else
#define _strdup strdup
char* _strrev(char* astr)
{
	char* forwards = astr;
	int len = strlen(astr);
	if (len > 1)
	{
		char* backwards = astr + len - 1;
		while (forwards < backwards)
		{
			char temp = *forwards;
			*forwards++ = *backwards;
			*backwards-- = temp;
		}
	}
	return astr;
}
#endif



int Topics_matches(char* wildTopic, char* topic)
{
	int rc = false;
	char *last1, *last2;
	char *pwild, *pmatch;

	if ((strchr(topic, '+') != NULL) || (strchr(topic, '#') != NULL))
	{
		Log(LOG_ERROR, Messages_get(12), topic);
		return false;
	}
	if (!Topics_isValidName(wildTopic))
	{
		Log(LOG_ERROR, Messages_get(13), wildTopic);
		return false;
	}
	if (!Topics_isValidName(topic))
	{
		Log(LOG_ERROR, Messages_get(13), topic);
		return false;
	}

	if (strcmp(wildTopic, MULTI_LEVEL_WILDCARD) == 0 || /* Hash matches anything... */
		strcmp(wildTopic, topic) == 0)
		return true;

	// We only match hash-first topics in reverse, for speed
	if (wildTopic[0] == MULTI_LEVEL_WILDCARD[0])
	{
		wildTopic = (char*)_strrev(_strdup(wildTopic));
		topic = (char*)_strrev(_strdup(topic));
	}
	else
	{
		wildTopic = (char*)_strdup(wildTopic);
		topic = (char*)_strdup(topic);
	}

	pwild = strtok_r(wildTopic, TOPIC_LEVEL_SEPARATOR, &last1);
	pmatch = strtok_r(topic, TOPIC_LEVEL_SEPARATOR, &last2);

	// Step through the subscription, level by level
	while (pwild != NULL)
	{
		// Have we got # - if so, it matches anything.
		if (strcmp(pwild, MULTI_LEVEL_WILDCARD) == 0)
		{
			rc = true;
			break;
		}
		// Nope - check for matches...
		if (pmatch != NULL)
		{
			if (strcmp(pwild, SINGLE_LEVEL_WILDCARD) != 0 && strcmp(pwild, pmatch) != 0)
				// The two levels simply don't match...
				break;
		}
		else
		{
			// Another place we can match Hash - if it evaluates to nothing at all...
			//pwild = strtok_r(NULL, TOPIC_LEVEL_SEPARATOR, &last1);
			//if ((pwild != NULL) && (strcmp(pwild, MULTI_LEVEL_WILDCARD) == 0))
			//	rc = true;
			break; // No more tokens to match against further tokens in the wildcard stream...
		}
		pwild = strtok_r(NULL, TOPIC_LEVEL_SEPARATOR, &last1);
		pmatch = strtok_r(NULL, TOPIC_LEVEL_SEPARATOR, &last2);
	}

	// All tokens up to here matched, and we didn't end in #. If there
	// are any topic tokens remaining, the match is bad, otherwise it was
	// a good match.
	if (pmatch == NULL && pwild == NULL)
		rc = true;

	//Now free the memory allocated in strdup()
	free(wildTopic);
	free(topic);

	return rc;
}                                                            /* end matches*/


#if defined(TOPICS_TEST)

int main(int argc, char *argv[])
{
	int i;

	struct
	{
		char* str;
	} tests0[] = {
		 "#", "jj",
		"+/a", "adkj/a",
		"+/a", "adsjk/adakjd/a", "a/+", "a/#", "#/a"
	};

	for (i = 0; i < sizeof(tests0)/sizeof(char*); ++i)
	{
		printf("topic %s, isValidName %d\n", tests0[i].str, Topics_isValidName(tests0[i].str));
		assert(Topics_isValidName(tests0[i].str) == 1);
	}

  struct
	{
		char* wild;
		char* topic;
		int result;
	} tests1[] = {
		{ "#", "jj" , 1},
		{ "+/a", "adkj/a", 1},
		{ "+/a", "adsjk/adakjd/a", 0},
		{ "+/+/a", "adsjk/adakjd/a", 1},
		{ "#/a", "adsjk/adakjd/a", 1},
		{ "test/#", "test/1", 1},
		{ "test/+", "test/1", 1},
	};

	for (i = 0; i < 7; ++i)
	{
		printf("wild: %s, topic %s, result %d\n", tests1[i].wild, tests1[i].topic,
						Topics_matches(_strdup(tests1[i].wild), _strdup(tests1[i].topic)));
		assert(Topics_matches(_strdup(tests1[i].wild), _strdup(tests1[i].topic)) == tests1[i].result);
	}


#if !defined(WIN32)
	struct
	{
		char* str;
		char* result;
	} tests2[] = {
		{ "#", "#" },
		{ "ab", "ba" },
		{ "abc", "cba" },
		{ "abcd", "dcba" },
		{ "abcde", "edcba" }
	};
	for (i = 0; i < 5; ++i)
	{
		printf("str: %s, _strrev %s\n", tests2[i].str, _strrev(_strdup(tests2[i].str)));
		assert(strcmp(tests2[i].result, _strrev(_strdup(tests2[i].str))) == 0);
	}
#endif

}

#endif
