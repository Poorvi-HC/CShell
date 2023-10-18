#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <limits.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include <netdb.h>
#include <arpa/inet.h>


// define the limits for the commands
#define MAX_ARGUMENTS 100        // max number of arguments in a command
#define MAX_COMMANDS 100         // max number of semi-colon seperated commands in one input
#define MAX_COMMAND_LEN 50000    // max length of string inputted to the shell at one go
#define MAX_FILENAME 256         // max length of filename 
#define MAX_INPUT_LEN 50000      // max length of input string (semi-colon seperated string)
#define MAX_TOKEN_LEN 200        // max length of a single space seeperated token from input
#define MAX_DIRS 2000            // max number of directories in the PATH variable
#define MAX_STAT_LEN 1024        // Maximum characters to read from the stat file
#define STAT_COUNT 52            // Number of space seperated values in a processes stat file
#define MAX_STORAGE 15           // max number of command stored
#define MAX_CHILD_COUNT 512      // max number of child processes allowed to execute
#define MAX_RESPONSE_SIZE 8192   // maximum size of HTTP response

// define the streams
#define STDIN 0
#define STDOUT 1
#define STDERR 2

// necessary permissions for shell
#define PERM 0644 // Default permission of file created with redirection

// ANSI color codes
#define ANSI_COLOR_RESET   "\x1b[0m"
#define ANSI_COLOR_GREEN   "\x1b[1;32m"  // For executables
#define ANSI_COLOR_BLUE    "\x1b[1;34m"  // For directories
#define ANSI_COLOR_WHITE   "\x1b[37m"  // For regular files

// Structure to child process information
struct pData
{
    pid_t pid;
    char pName[MAX_FILENAME + 1];
    char command[MAX_COMMAND_LEN + 1];
    char status[30];
};

#endif