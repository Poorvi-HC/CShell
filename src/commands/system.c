// file contains implementation of the functions which allows processes to run in the
// foreground and background

// cutsom header files
#include "system.h"

// Variables declared elsewhere
extern char INVOC_LOC[];
extern int exitCode;
extern struct pData childArray[];
extern int childCount;
extern char SHELL_NAME[];
extern char foreProcess;
extern char chld_argument[];
extern char fgFlag;
extern struct pData currProcess;

// global variables
extern double fgExecutionTime;


// Method to implement background process implementation
int background(char *args[], char* COMMAND)
{
    // if the location of the executable is relative to ~
    // make path absolute
    if (args[0][0] == '~')
    {
        char path[PATH_MAX + 1];
        sprintf(path, "%s/%s", INVOC_LOC, args[0] + 1);
        args[0] = path;
    }
    // check if there are more '&' in the command
    for (int i = 0; args[i] != NULL; i++)
    {
        if (!strcmp(args[i], "&"))
        {
            args[i] = NULL;
            break;
        }
    }

    // fork the process
    // handle error
    pid_t pid = fork();
    if (pid < 0)
    {
        exitCode = -1;
        return -1;
    }

    // In child process
    if (pid == 0)
    {
        // Print the PID of the newly created background process
        printf("%d\n", getpid());

        // if number of children is less than the
        // maximum number of child processes allowed to run
        // then run in the background
        if (childCount < MAX_CHILD_COUNT)
        {
            // move the process to a new process group
            // to send it to the background
            setpgid(0, 0);

            // Check if given command is valid
            // handle errors
            if (execvp(args[0], args))
            {
                fprintf(stderr, "%s: command not found\n", args[0]);
                exit(1);
            }
        }
        // if no more space is available
        else
        {
            fprintf(stderr, "No more space available\n");
            exit(1);
        }
    }
    // insert child process in parent
    else
    {
        insertChildProcess(pid, args[0], COMMAND, "Running");
    }
}


// method to implement foreground process 
int foreground(char* args[], char* COMMAND)
{
    fgFlag = 1;

    // if path relative to ~
    // convert to absolute
    if (args[0][0] == '~')
    {
        char path[PATH_MAX + 1];
        sprintf(path, "%s/%s", INVOC_LOC, args[0] + 1);
        args[0] = path;
    }
    
    struct timespec start, end;

    clock_gettime(CLOCK_MONOTONIC, &start);

    // fork process
    // handle error
    pid_t pid = fork();
    foreProcess = 1;
    if(pid < 0)
    {
        exitCode = -1;
        return -1;
    }

    // In child process execute the given command
    if(pid == 0)
    {
        // if number of children is less than MAX_CHILD_COUNT
        // then run in foreground
        if (execvp(args[0], args))
        {
            fprintf(stderr, "%s Error: Command not found: %s\n", SHELL_NAME, args[0]) ;
            exit(1);
        }
    }

    // In parent process 
    // wait for child to complete
    else
    {
        int sit;
        if(waitpid(pid, &sit, WUNTRACED) > 0)
        {
            if(WIFSTOPPED(sit)){
                insertChildProcess(pid, args[0], COMMAND, "Stopped");
            }
        }
        strcpy(chld_argument, args[0]);
        // Record the end time and calculate execution time
        clock_gettime(CLOCK_MONOTONIC, &end);  // Record end time

        // Calculate elapsed time in seconds
        fgExecutionTime = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

        foreProcess = 0;
    }
    return 0;
}

