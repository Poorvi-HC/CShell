// this has the implementation of signal handling
// we are handling keyboard interupts and signals from child processes
// SIGINT, SIGCHLD, SIGTSTP
// Ctrl - C -> SIGINT
// Ctrl - Z -> SIGTSTP
// Child process termination -> SIGCHLD
// Ctrl- D -> SIGQUIT

// include cutsom headers
#include "sig.h"

// variables declared elsewhere
extern struct pData childArray[];
extern int childCount;
extern char exitCode;
extern char PString[];
extern char INVOC_LOC[];
extern char TEMP_BUFFER[];
extern char foreProcess;
extern char pinged;

//method to handle SIGCHLD signal
//handles the termination of child processes
void sigchldHandler(int sig)
{
    // printf("enters sigchild handler function\n");
    // variables to be used
    pid_t pid;
    int stat;
    int kill = 0;
    struct pData data;

    while ((pid = waitpid(-1, &stat, WNOHANG)) > 0)
    {
        stat = WIFEXITED(stat);
        char status[20];

        // get the data of the child process
        data = get_data_by_pid(pid);

        // child not in pool
        if(data.pid == -1)
        {   
            fprintf(stderr,"Process with PID: %d not in child pool\n", pid);
            handlePError(0);
            exitCode = -1;
            kill = 1;
        }
        // pid in child process list
        else
        {
            // termination message
            strcpy(status,stat != 0 ? "normally" : "abnormally");
            sprintf(TEMP_BUFFER, "\n%s with pid %d exited %s\n", data.command, (int)pid, status);

            // write into standard error
            write(STDERR, TEMP_BUFFER, strlen(TEMP_BUFFER));
            // remove the child from the pool
            char found = 0;
            // If a child process that is in the middle of the array is killed, the processes
            // ahead of it are moved back one space to keep them contigeous
            for (int i = 0; i < childCount; i++)
            {
                if (childArray[i].pid == pid && !found)
                    found = 1;
                if (found)
                {
                    if (i == childCount - 1)
                        childArray[i].pid = -1;
                    else
                    {
                        childArray[i].pid = childArray[i + 1].pid;
                        strcpy(childArray[i].pName, childArray[i + 1].pName);
                        strcpy(childArray[i].status, childArray[i + 1].status);
                        strcpy(childArray[i].command, childArray[i + 1].command);
                    }
                }
            }
            childCount--;
        }
        // printf("killing\n");
        kill++;
        exitCode = stat ? -1 : 1;
    }
    // If a process from the pool was kill
    if (kill)
    {
        // printf("index 1\n");
        write(STDOUT, "\n", 1);
        generatePString(0, PString, INVOC_LOC);
        pinged = 0;
        write(STDOUT, PString, strlen(PString));
    }
}

// method to handle Ctrl-C from user
// interupts the currently running foreground process 
// has no effect if no foreground process in running
void sigintHandler(int sig)
{
    if(foreProcess)
        printf("\nSIGNAL: Stopping...\n");
    write(STDOUT, "\n", 1);
    if(!foreProcess){
        // printf("index 2\n");
        pinged = 0;
        write(STDOUT, PString, strlen(PString));
    }
}

// method to handle Ctrl-Z from user
// push (if any foreground process is running) into the background 
// change its state from "Running" to "Stopped"
// it has no effect if no foreground process is running
void sigstpHandler(int sig)
{
    if(foreProcess)
        printf("\nSIGNAL: Moving to background...\n");
    write(STDOUT, "\n", 1);
    if(!foreProcess){
        // printf("index 3\n");
        pinged = 0;
        write(STDOUT, PString, strlen(PString));
    }
}


// method to handle Ctrl-D from user
// log out of the shell (after killing all the child processes)
void sigquitHandler(int sig)
{
    printf("\nSIGNAL: Logging out...\n");
    write(STDOUT, "\n", 1);
    exit(0);
}