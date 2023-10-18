#ifndef SIG_H
#define SIG_H

// include custom headers
#include "../definitions.h"
#include "../utilities.h"

// function declarations
void sigintHandler(int sig);
void sigstpHandler(int sig);
void sigchldHandler(int sig);
void sigquitHandler(int sig);

#endif