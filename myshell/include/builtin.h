#pragma once // to support backword compatibility
#ifndef __BUILTIN_H__
#define __BUILTIN_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>
#include <errno.h>
#include <limits.h>

#include <readline/history.h>
#include <readline/history.h>

#define BUILTINS 4 // as for now, i have only 2 builtin commands
extern char* builtinCmd[BUILTINS]; // array of builtin commands
extern int (*builtinCmdPtr[BUILTINS])(int, char**); // array of function pointer to the internal commands

int execBuiltin(int cmdNo, char **argv);
int isBuiltin(const char* cmd);

int myexit(int argc, char** argv);
int mycd(int argc, char** argv);
int myjobs(int argc, char **argv);
int myhelp(int argc, char **argv);

#endif
