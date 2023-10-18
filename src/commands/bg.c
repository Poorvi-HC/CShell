// this file contains implementation of bg process
// bg changes state of a stopped background process to running in the background
// If a process with given pid is not found, an error is thrown
// syntax
//     bg <pid>

// include custom header files
#include "bg.h"

// variables defined elsewhere
extern char exitCode;

// method to run the bg command
int run_bg(char* args[])
{
    if (bg(args))
        handlePError(0);
    return 0;
}

int bg(char* args[])
{
    int count_args = 0;
    while(args[count_args] != NULL)
        count_args++;
    
    if (count_args != 2)
    {
        fprintf(stderr, "bg: Invalid number of arguments. SYNTAX: bg <pid>\n");
        handlePError(0);
        exitCode = -1;
        return 1;
    }

    // GET PID OF THE PROCESS
    pid_t pid = atoi(args[1]);
    if(pid < 0)
    {
        fprintf(stderr, "bg: Enter Positive integer for process PID, SYNTAX: bg <pid>\n");
        handlePError(0);
        exitCode = -1;
        return 1;
    }

    // check if pid is not a floating point number
    for(int i = 0;i<strlen(args[1]);i++){
        if(isdigit(args[1][i]) == 0){
            fprintf(stderr, "fg: Enter Positive integer for process ID, SYNTAX: fg <pid>\n");
            handlePError(0);
            exitCode = -1;
            return 1;
        }
    }

    // get the data of the child process with the given pid
    struct pData data = get_data_by_pid(pid);

    // error handling
    if(data.pid == -1)
    {
        // such a process doesn't exist
        fprintf(stderr, "bg: Process with PID: %d not in child pool\n", pid);
        handlePError(0);
        exitCode = -1;
        return 1;
    }

    // if the process is not stopped, throw an error
    if(strcmp(data.status,"Stopped"))
    {
        fprintf(stderr, "bg: Process with PID: %d is not stopped\n", pid);
        handlePError(0);
        exitCode = -1;
        return 1;
    }

    // to continue the process pass the SIGCONT signal to the process
    int retval = kill(pid, SIGCONT);
    return retval;
}