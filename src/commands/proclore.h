#ifndef PROCLORE_H
#define PROCLORE_H

// include custom headers
#include "../definitions.h"
#include "../utilities.h"

int proclore(char *arg[]);
int runProclore(char* args[]);
int isForeground(pid_t pid);

#endif