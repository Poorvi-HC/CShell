#ifndef PEEK_H
#define PEEK_H

// include custom files
#include "../definitions.h"
#include "../utilities.h"

// function declarations
int peek(char* args[]);
void runPeek(char* args[]);
int compare(const void *a, const void *b);
int printFileDetails(char* dir_name, struct stat fileStat, int columnWidth[]);
void printFilePermissions(mode_t bits);

#endif