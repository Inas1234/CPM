#ifndef PACKAGE_MANAGER_H
#define PACKAGE_MANAGER_H

int install_package(const char *package_name);

int uninstall_package(const char *package_name);

int list_installed_packages(void);

void ensure_directory_exists(const char *path);

int save_package_lock(void);

int load_package_lock(void);

#endif 
