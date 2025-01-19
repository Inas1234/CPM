#include "package_manager.h"
#include "network.h"
#include "json_parser.h"
#include "downloader.h"
#include "thread_pool.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cjson/cJSON.h>
#include <pthread.h>

#define NODE_MODULES_DIR "./node_modules"
#define LOCK_FILE "./package-lock.json"

static cJSON *lock_file_root = NULL;
static pthread_mutex_t lock_file_mutex = PTHREAD_MUTEX_INITIALIZER;

void update_lock_file_safe(const char *package_name, const char *version, cJSON *dependencies) {
    pthread_mutex_lock(&lock_file_mutex);
    update_lock_file(package_name, version, dependencies);
    pthread_mutex_unlock(&lock_file_mutex);
}

int save_package_lock_safe(void) {
    pthread_mutex_lock(&lock_file_mutex);
    int result = save_package_lock();
    pthread_mutex_unlock(&lock_file_mutex);
    return result;
}

int load_package_lock(void) {
    FILE *file = fopen(LOCK_FILE, "r");
    if (!file) {
        lock_file_root = cJSON_CreateObject();
        cJSON_AddItemToObject(lock_file_root, "dependencies", cJSON_CreateObject());
        return 0;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    char *buffer = (char *)malloc(file_size + 1);
    fread(buffer, 1, file_size, file);
    fclose(file);
    buffer[file_size] = '\0';

    lock_file_root = cJSON_Parse(buffer);
    free(buffer);

    if (!lock_file_root) {
        fprintf(stderr, "Error parsing package-lock.json.\n");
        return 1;
    }

    return 0;
}

int save_package_lock(void) {
    if (!lock_file_root) return 1;

    char *json_string = cJSON_Print(lock_file_root);
    if (!json_string) return 1;

    FILE *file = fopen(LOCK_FILE, "w");
    if (!file) {
        fprintf(stderr, "Failed to open package-lock.json for writing.\n");
        free(json_string);
        return 1;
    }

    fwrite(json_string, 1, strlen(json_string), file);
    fclose(file);
    free(json_string);

    return 0;
}

void update_lock_file(const char *package_name, const char *version, cJSON *dependencies) {
    cJSON *dependencies_root = cJSON_GetObjectItemCaseSensitive(lock_file_root, "dependencies");
    if (!dependencies_root) {
        dependencies_root = cJSON_CreateObject();
        cJSON_AddItemToObject(lock_file_root, "dependencies", dependencies_root);
    }

    cJSON *package_entry = cJSON_CreateObject();
    cJSON_AddStringToObject(package_entry, "version", version);

    if (dependencies) {
        cJSON_AddItemToObject(package_entry, "dependencies", cJSON_Duplicate(dependencies, 1));
    }

    cJSON_AddItemToObject(dependencies_root, package_name, package_entry);
}

void ensure_directory_exists(const char *path) {
    char command[1024];
    snprintf(command, sizeof(command), "mkdir -p %s", path);
    system(command);
}

void install_dependency_task(void *arg) {
    const char *dependency_name = (const char *)arg;

    install_package(dependency_name);

    pthread_mutex_lock(&pool.lock);

    // // pool.active_tasks--;
    // if (pool.active_tasks == 0 && !pool.task_queue_head) {
    //     pthread_cond_signal(&pool.all_done);
    // }

    // pthread_mutex_unlock(&pool.lock);

    free((void *)dependency_name);
}

int install_package(const char *package_name) {
    char url[1024];
    snprintf(url, sizeof(url), "https://registry.npmjs.org/%s/latest", package_name);

    HttpResponse response;
    if (http_get(url, &response) != 0) {
        fprintf(stderr, "Failed to fetch package metadata from %s\n", url);
        return 1;
    }

    char *name = NULL, *version = NULL;
    if (!parse_package_metadata(response.data, &name, &version)) {
        fprintf(stderr, "Failed to parse package metadata.\n");
        free_http_response(&response);
        return 1;
    }

    printf("Installing package: %s@%s\n", name, version);

    char tarball_url[1024];
    snprintf(tarball_url, sizeof(tarball_url),
             "https://registry.npmjs.org/%s/-/%s-%s.tgz",
             name, name, version);

    char tarball_path[1024], extract_dir[1024];
    snprintf(tarball_path, sizeof(tarball_path), "%s/%s.tgz", NODE_MODULES_DIR, name);
    snprintf(extract_dir, sizeof(extract_dir), "%s/%s", NODE_MODULES_DIR, name);

    ensure_directory_exists(NODE_MODULES_DIR);

    if (!download_file(tarball_url, tarball_path)) {
        fprintf(stderr, "Failed to download package tarball: %s\n", tarball_url);
        free(name);
        free(version);
        free_http_response(&response);
        return 1;
    }

    if (!extract_tarball(tarball_path, extract_dir)) {
        fprintf(stderr, "Failed to extract package: %s\n", tarball_path);
        free(name);
        free(version);
        free_http_response(&response);
        return 1;
    }

    if (!delete_archive(tarball_path)) {
        fprintf(stderr, "Failed to delete archive: %s\n", tarball_path);
        free(name);
        free(version);
        free_http_response(&response);
        return 1;
    }

    char *dependencies = parse_package_dependencies(response.data);
    cJSON *dependencies_json = cJSON_Parse(dependencies);
    if (dependencies_json) {
        printf("Installing dependencies for %s@%s:\n", name, version);
        update_lock_file_safe(name, version, dependencies_json);

        cJSON *dep;
        cJSON_ArrayForEach(dep, dependencies_json) {
            const char *dep_name = dep->string;
            if (dep_name) {
                printf("Queueing dependency: %s\n", dep_name);
                thread_pool_add_task(install_dependency_task, strdup(dep_name));
            }
        }

        // thread_pool_wait(); // Wait for all tasks to complete

        cJSON_Delete(dependencies_json);
        free(dependencies);

    } else {
        printf("No dependencies found for %s@%s.\n", name, version);
        update_lock_file_safe(name, version, NULL);

        // pthread_mutex_lock(&pool.lock);
        // pool.active_tasks--; // Decrement active tasks for tasks with no dependencies
        // if (pool.active_tasks == 0 && !pool.task_queue_head) {
        //     pthread_cond_signal(&pool.all_done);
        //     printf("[DEBUG] No dependencies. Signaling main thread.\n");
        // }
        // pthread_mutex_unlock(&pool.lock);
    }

    save_package_lock_safe();

    printf("Package %s@%s installed successfully.\n", name, version);

    free(name);
    free(version);
    free_http_response(&response);
    return 0;
}

int uninstall_package(const char *package_name) {
    cJSON *dependencies_root = cJSON_GetObjectItemCaseSensitive(lock_file_root, "dependencies");
    if (!dependencies_root) {
        fprintf(stderr, "No installed packages found.\n");
        return 1;
    }

    cJSON *package_entry = cJSON_GetObjectItemCaseSensitive(dependencies_root, package_name);
    if (!package_entry) {
        fprintf(stderr, "Package %s is not installed.\n", package_name);
        return 1;
    }

    cJSON *deps = cJSON_GetObjectItemCaseSensitive(package_entry, "dependencies");
    if (deps && cJSON_IsObject(deps)) {
        cJSON *dep;
        cJSON_ArrayForEach(dep, deps) {
            uninstall_package(dep->string); 
        }
    }

    char path[1024];
    snprintf(path, sizeof(path), "%s/%s", NODE_MODULES_DIR, package_name);
    char command[1024];
    snprintf(command, sizeof(command), "rm -rf %s", path);
    system(command);

    cJSON_DeleteItemFromObject(dependencies_root, package_name);
    save_package_lock_safe();

    printf("Package %s uninstalled successfully.\n", package_name);
    return 0;
}

int list_installed_packages(void) {
    DIR *dir = opendir(NODE_MODULES_DIR);
    if (!dir) {
        fprintf(stderr, "No packages installed.\n");
        return 1;
    }

    struct dirent *entry;
    printf("Installed packages:\n");
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            printf("- %s\n", entry->d_name);
        }
    }

    closedir(dir);
    return 0;
}
