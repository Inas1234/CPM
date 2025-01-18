#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "network.h"
#include "json_parser.h"
#include "downloader.h"
#include "package_manager.h"

int main(int argc, char *argv[]) {
    if (load_package_lock() != 0) {
        fprintf(stderr, "Failed to load package-lock.json.\n");
        return 1;
    }

    ParsedCommand cmd = parse_command(argc, argv);

    switch (cmd.command) {
        case COMMAND_INSTALL:
            if (!cmd.arg) {
                printf("Error: Please specify a package to install.\n");
                return 1;
            }
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
            if (uninstall_package(cmd.arg) != 0) {
                fprintf(stderr, "Failed to uninstall package: %s\n", cmd.arg);
                return 1;
            }
            break;

        case COMMAND_LIST:
            if (list_installed_packages() != 0) {
                fprintf(stderr, "Failed to list installed packages.\n");
                return 1;
            }
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

    save_package_lock();

    return 0;
}
