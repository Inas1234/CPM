#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "network.h"
#include "json_parser.h"
#include "downloader.h"
#include "package_manager.h"
#include "thread_pool.h"

int main(int argc, char *argv[]) {
    if (!thread_pool_init(8)) {
        fprintf(stderr, "Failed to initialize thread pool.\n");
        return 1;
    }

    if (load_package_lock() != 0) {
        fprintf(stderr, "Failed to load package-lock.json.\n");
        thread_pool_destroy();
        return 1;
    }

    ParsedCommand cmd = parse_command(argc, argv);
    int result = 0;

    switch (cmd.command) {
        case COMMAND_INSTALL:
            if (!cmd.arg) {
                printf("Error: Please specify a package to install.\n");
                result = 1;
                break;
            }
            printf("Installing package: %s\n", cmd.arg);
            if (install_package(cmd.arg) != 0) {
                fprintf(stderr, "Failed to install package: %s\n", cmd.arg);
                result = 1;
            }
            else {
                thread_pool_wait();
            }
            break;

        case COMMAND_UNINSTALL:
            if (!cmd.arg) {
                printf("Error: Please specify a package to uninstall.\n");
                result = 1;
                break;
            }
            printf("Uninstalling package: %s\n", cmd.arg);
            if (uninstall_package(cmd.arg) != 0) {
                fprintf(stderr, "Failed to uninstall package: %s\n", cmd.arg);
                result = 1;
            }
            break;

        case COMMAND_LIST:
            printf("Listing installed packages:\n");
            if (list_installed_packages() != 0) {
                fprintf(stderr, "Failed to list installed packages.\n");
                result = 1;
            }
            break;

        case COMMAND_HELP:
            print_help();
            break;

        case COMMAND_UNKNOWN:
        default:
            printf("Error: Unknown command.\n");
            print_help();
            result = 1;
            break;
    }

    save_package_lock();
    thread_pool_destroy();


    return result;
}
