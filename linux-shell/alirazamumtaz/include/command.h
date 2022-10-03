
#pragma once // to support backword compatibility
#ifndef __COMMAND_H__
#define __COMMAND_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>
#include <readline/history.h>
#include <readline/history.h>

#define MAX_LEN 512
#define MAXARGS 10
#define ARGLEN 30

int** pipeArray; // array of pipes
int pipeCount; // number of pipes
int curr_history_length;

typedef struct _command_t { 
    int argc;
    char** argv;
    int instream, outstream, errorstream;
    int pipein, pipeout;
} command_t;

char* read_command(char* prompt);
command_t* tokenize(char* cmdline);
command_t** parse(char* cmdline, int* no_of_commands);
int execute(command_t *cmd);
char** parse_atomic_command(char* string, int* count, char* seperator);
void set_pipe(command_t* command, int j, int pipe_sperterated_count);
void set_ioredirect(command_t* command, int j, int pipe_sperterated_count);

#endif
