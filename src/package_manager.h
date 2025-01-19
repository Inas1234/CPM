#ifndef PACKAGE_MANAGER_H
#define PACKAGE_MANAGER_H

#include <cjson/cJSON.h>

int install_package(const char *package_name);

int uninstall_package(const char *package_name);

int list_installed_packages(void);

void ensure_directory_exists(const char *path);

int save_package_lock(void);

int load_package_lock(void);

void update_lock_file(const char *package_name, const char *version, cJSON *dependencies);

void install_dependency_task(void *arg);

int save_package_lock_safe(void);

void update_lock_file_safe(const char *package_name, const char *version, cJSON *dependencies);

#endif 
