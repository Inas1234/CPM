#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "parser.h"
#include "network.h"
#include "json_parser.h"
#include "downloader.h"
#include "package_manager.h"
#include "thread_pool.h"
#include "logger.h"

int main(int argc, char *argv[]) {

    logger_init(1);

    int num_threads = sysconf(_SC_NPROCESSORS_ONLN);
    if (!thread_pool_init(num_threads)) {
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
                // printf("Error: Please specify a package to install.\n");
                log_message(LOG_LEVEL_ERROR, "Please specify a package to install");
                result = 1;
                break;
            }
            // printf("Installing package: %s\n", cmd.arg);
            log_message(LOG_LEVEL_HEADER, "Installing package: %s", cmd.arg);
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
                // printf("Error: Please specify a package to uninstall.\n");
                log_message(LOG_LEVEL_ERROR, "Please specify a package to uninstall");
                result = 1;
                break;
            }
            // printf("Uninstalling package: %s\n", cmd.arg);
            log_message(LOG_LEVEL_HEADER, "Uninstalling package: %s", cmd.arg);
            if (uninstall_package(cmd.arg) != 0) {
                // fprintf(stderr, "Failed to uninstall package: %s\n", cmd.arg);
                log_message(LOG_LEVEL_ERROR, "Failed to uninstall package: %s", cmd.arg);
                result = 1;
            }
            break;

        case COMMAND_LIST:
            // printf("Listing installed packages:\n");
            log_message(LOG_LEVEL_HEADER, "Listing installed packages");
            if (list_installed_packages() != 0) {
                // fprintf(stderr, "Failed to list installed packages.\n");
                log_message(LOG_LEVEL_ERROR, "Failed to list installed packages");
                result = 1;
            }
            break;

        case COMMAND_HELP:
            print_help();
            break;

        case COMMAND_UNKNOWN:
        default:
            // printf("Error: Unknown command.\n");
            log_message(LOG_LEVEL_ERROR, "Unknown command");
            print_help();
            result = 1;
            break;
    }

    save_package_lock();
    thread_pool_destroy();


    return result;
}
