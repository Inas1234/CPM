#include "parser.h"
#include <stdio.h>
#include <string.h>

ParsedCommand parse_command(int argc, char*argv[]){
    ParsedCommand result = {COMMAND_UNKNOWN, NULL};
    if (argc < 2){
        result.command = COMMAND_HELP;
        return result;
    }

    const char* command = argv[1];

    if (strcmp(command, "install") == 0) {
        result.command = COMMAND_INSTALL;
    } else if (strcmp(command, "uninstall") == 0) {
        result.command = COMMAND_UNINSTALL;
    } else if (strcmp(command, "list") == 0) {
        result.command = COMMAND_LIST;
    } else if (strcmp(command, "help") == 0) {
        result.command = COMMAND_HELP;
    } else {
        result.command = COMMAND_UNKNOWN;
    }

 
    if (argc >= 3 && command_requires_argument(result.command)) {
        result.arg = argv[2];
    }

    return result;

}


bool command_requires_argument(Command command){
    return command == COMMAND_INSTALL || command == COMMAND_UNINSTALL;
}

void print_help(){
    printf("Usage: package-manager <command> [argument]\n");
    printf("Commands:\n");
    printf("  install <package>  Install a package\n");
    printf("  uninstall <package>  Uninstall a package\n");
    printf("  list  List installed packages\n");
    printf("  help  Display this help message\n");
}