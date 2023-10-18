#ifndef UTILITIES_H
#define UTILITIES_H


void clear();
int generatePString(char init, char* PString, char* INVOC_LOC);
void processInputString(char *INP_STRING);
void psError(int ps_state);
void shortenPathSubDir(char* INVOC_LOC, char* PWD);
void handlePError(int quit);
void processInputString(char* INPUT_STRING);
void processCommand(char* COMMAND_STRING);
void managePipes(char* CLEAN_COMMAND);
void execCommand(char *COMMAND);
long long int max(long long int a, long long int b);
int digitCount(long long x);
void initChildren();
void initChildHandlers();
void initStpHandlers();
void initIntHandlers();
void sigintHandler(int sig);
void sigstpHandler(int sig);
void sigchldHandler(int sig);
void process_amp(char* COMMAND_STRING);
struct pData get_data_by_pid(pid_t pid);
void removeChildProcess(pid_t pid);
int insertChildProcess(int pid, char *processName, char *command, char* status);



#endif
