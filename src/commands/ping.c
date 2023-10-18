// ping implementation is included in this file
// ping is used to signals to processes
// command structure:
    // ping <pid> <signal_number>
// signal_number's modulo with 32 gives the signal code

// include custom header files
#include "ping.h"

// variables defined elsewhere
extern char exitCode;
extern struct pData childArray[];
extern int childCount;

// method to run the ping command
int runPing(char* args[])
{
    if (ping(args)){
        printf("no sucess\n");
        handlePError(0);
        return 1;
    }
    return 0;
}

// method to implement ping
int ping(char* args[])
{
    int idx = 0;
    while(args[idx] != NULL)
        idx++;

    if (idx != 3)
    {
        fprintf(stderr, "ping: Invalid number of arguments\n");
        handlePError(0);
        exitCode = -1;
        return 1;
    }

    // get pid and signal number
    pid_t pid = (pid_t)atoi(args[1]);
    if(pid <= 0)
    {
        fprintf(stderr, "ping: Enter Positive integer for job PID, SYNTAX: ping <pid> <signal_number>\n");
        handlePError(0);
        exitCode = -1;
        return 1;
    }
    int signal_number = atoi(args[2]);
    if(signal_number <= 0)
    {
        fprintf(stderr, "ping: Enter Positive integer for signal number, SYNTAX: ping <pid> <signal_number>\n");
        handlePError(0);
        exitCode = -1;
        return 1;
    }

    // signal_number is modulo 32
    signal_number %= 32;

    // printf("%d %d\n", pid, signal_number);

    // check if PID exists
    int found = 0;
    for(int i = 0;i < childCount;i++)
    {
        if(childArray[i].pid == pid)
        {
            found = 1;
            break;
        }
    }

    if(!found)
    {
        fprintf(stderr, "ping: Process with PID %d does not exist\n", pid);
        handlePError(0);
        exitCode = -1;
        return 1;
    }

    // send signal to process
    // signals - 1-32
    // SIGINT, SIGKILL, SIGCHLD, SIGALRM, SIGTERM, etc...
    int flg = kill(pid, signal_number);
    if(flg < 0)
    {
        fprintf(stderr, "ping: Error in signalling process %s: with pid %d\n", childArray[pid].command, pid);
        handlePError(0);
        exitCode = -1;
        return 1;
    }
    return 0;
}
