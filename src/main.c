#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "network.h"
#include "json_parser.h"
#include "downloader.h"

#define NODE_MODULES_DIR "./node_modules"

void ensure_directory_exists(const char *path) {
    char command[1024];
    snprintf(command, sizeof(command), "mkdir -p %s", path);
    system(command);
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

    printf("Package %s@%s installed successfully.\n", name, version);

    free(name);
    free(version);
    free_http_response(&response);
    return 0;
}


int main(int argc, char *argv[]) {
    ParsedCommand cmd = parse_command(argc, argv);

    switch (cmd.command) {
        case COMMAND_INSTALL:
            if (!cmd.arg) {
                printf("Error: Please specify a package to install.\n");
                return 1;
            }
            printf("Installing package: %s\n", cmd.arg);
            if (install_package(cmd.arg) != 0) {
                fprintf(stderr, "Failed to install package: %s\n", cmd.arg);
                return 1;
            }
            break;

        case COMMAND_UNINSTALL:
            if (!cmd.arg) {
                printf("Error: Please specify a package to uninstall.\n");
                return 1;
            }
            printf("Uninstalling package: %s\n", cmd.arg);
            break;

        case COMMAND_LIST:
            printf("Listing installed packages...\n");
            break;

        case COMMAND_HELP:
            print_help();
            break;

        case COMMAND_UNKNOWN:
        default:
            printf("Error: Unknown command.\n");
            print_help();
            return 1;
    }

    return 0;
}
