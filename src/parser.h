#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>

typedef enum {
    COMMAND_INSTALL,
    COMMAND_UNINSTALL,
    COMMAND_LIST,
    COMMAND_HELP,
    COMMAND_UNKNOWN    
} Command;

typedef struct {
    Command command;    
    const char *arg;    
} ParsedCommand;


ParsedCommand parse_command(int argc, char *argv[]);
bool command_requires_argument(Command command);
void print_help();


#endif