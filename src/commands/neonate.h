#ifndef NEONATE_H
#define NEONATE_H

// include custom header files
#include "../definitions.h"
#include "../utilities.h"

// function declarations
int runNeonate(char* args[]);
int neonate(char* args[]);
void disableRawMode();
void enableRawMode();
void die(const char *s);
int kbhit();


#endif