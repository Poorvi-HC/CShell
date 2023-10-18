#ifndef SYSTEM_H
#define SYSTEM_H

// include inbuilt libraries
#include <signal.h>

// include custom meaders
#include "../definitions.h"
#include "../utilities.h"

// include function declarations
int background(char* args[], char* COMMAND);
int foreground(char* args[], char* COMMAND);

#endif