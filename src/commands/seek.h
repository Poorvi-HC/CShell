#ifndef SEEK_H
#define SEEK_H

// include custom header files
#include "../definitions.h"
#include "../utilities.h"

// function declarations
int seek(char* args[]);
int runSeek(char *args[]);
void recurse_for_File(char file[PATH_MAX], char dir[PATH_MAX], char string[PATH_MAX + 1], int FlagF, int FlagD);


#endif