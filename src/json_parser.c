#include "json_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cjson/cJSON.h>

bool parse_package_metadata(const char *json, char **name, char **version) {
    cJSON *root = cJSON_Parse(json);
    if (!root) {
        fprintf(stderr, "Error parsing JSON: %s\n", cJSON_GetErrorPtr());
        return false;
    }

    cJSON *name_item = cJSON_GetObjectItemCaseSensitive(root, "name");
    cJSON *version_item = cJSON_GetObjectItemCaseSensitive(root, "version");

    if (cJSON_IsString(name_item) && cJSON_IsString(version_item)) {
        *name = strdup(name_item->valuestring);
        *version = strdup(version_item->valuestring);
    } else {
        fprintf(stderr, "Error: Invalid or missing 'name' or 'version' fields\n");
        cJSON_Delete(root);
        return false;
    }

    cJSON_Delete(root);
    return true;
}

char *parse_package_dependencies(const char *json) {
    cJSON *root = cJSON_Parse(json);
    if (!root) {
        fprintf(stderr, "Error parsing JSON: %s\n", cJSON_GetErrorPtr());
        return NULL;
    }

    cJSON *dependencies = cJSON_GetObjectItemCaseSensitive(root, "dependencies");
    if (!cJSON_IsObject(dependencies)) {
        fprintf(stderr, "No dependencies found.\n");
        cJSON_Delete(root);
        return strdup("{}");
    }

    char *dependencies_str = cJSON_Print(dependencies); 
    cJSON_Delete(root);

    return dependencies_str; 
}
