#ifndef PASTEVENTS_H
#define PASTEVENTS_H

// include custom headers
#include "../definitions.h"
#include "../utilities.h"

int pastEventsPurge(char *args[]);
int pastEventsExec(char* args[]);
void savePastEvents();
int loadPastEvents(char* COMMAND);
void readPastEvents();
void move_uphill();
int pastevents(char* args[]);
int runPastEvents(char* args[]);

#endif