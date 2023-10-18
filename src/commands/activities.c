// this file has the implementation of activities

// activities prints the list of all the processes spawned by the shell

// custom header files
#include "activities.h"

// variables defined elsewhere
extern struct pData childArray[];
extern char exitCode;
extern int childCount;

int runActivities(char* args[])
{
    if (activities(args))
        handlePError(0);
    return 0;
}


// compare function used to sort the childArray
int compare_pdata(const void* a, const void* b)
{
    struct pData* p1 = (struct pData*)a;
    struct pData* p2 = (struct pData*)b;
    return p1->pid - p2->pid;
}

// method used to implement activities
int activities(char* args[])
{
    // parameters of the process
    pid_t pid;
    char* command;
    char status[100];
    char progress;

    // process variables
    char PATH[20];
    char stat[100];

    FILE* processStat;

    // sort childArray in lexicographically order of pid
    qsort(childArray, childCount, sizeof(struct pData), compare_pdata);

    for(int i = 0;i < childCount;i++)
    {
        // check for status of each process
        pid = childArray[i].pid;
        command = childArray[i].command;

        sprintf(PATH, "/proc/%d/stat", pid);
        processStat = fopen(PATH, "r");
        if(processStat == NULL)
        {
            errno = ESRCH;
            exitCode = -1;
            return -1;
        }
        fread(stat, 100, 1, processStat);
        fclose(processStat);

        // get the status of the process
        // the 3rd attribute gives the status of the process
        char* temp = strtok(stat, " ");
        temp = strtok(NULL, " ");
        temp = strtok(NULL, " ");

        char progress = temp[0];
        bool isRunning = progress == 'S';

        if (isRunning)
        {
            strcpy(status, "Running");
        }
        else
        {
            strcpy(status, "Stopped");
        }

        printf("%d : %s - %s\n", pid, command, status);
    }
    return 0;
}
