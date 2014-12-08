/* (C) Copyright IBM Corp. 2007, 2008
  Author: Ian Craggs
*/

#if !defined(TOPICS_H)
#define TOPICS_H

#include "NanoBroker.h"

static const char* TOPIC_LEVEL_SEPARATOR = "/";
static const char* SINGLE_LEVEL_WILDCARD = "+";
static const char* MULTI_LEVEL_WILDCARD = "#";

int Topics_isValidName(char* aName);

int Topics_matches(char* wildTopic, char* topic);

#endif // TOPICS_H
