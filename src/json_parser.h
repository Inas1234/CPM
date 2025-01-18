#ifndef JSON_PARSER_H
#define JSON_PARSER_H

#include <stdbool.h>

bool parse_package_metadata(const char *json, char **name, char **version);

char *parse_package_dependencies(const char *json);

#endif 
