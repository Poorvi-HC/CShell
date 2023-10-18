// this file contains the implementation of the ffg command
// fg - foreground process
// fg is used to bring a background process to the foreground
// gives control of the terminal to the process
// command structure:
//     fg <pid>

// include custom header files
#include "fg.h"

// variables defined elsewhere
extern char exitCode;
extern struct pData childArray[];
extern int childCount;
extern char foreProcess;

// method to run the fg command
int run_fg(char* args[])
{
    if (fg(args))
        handlePError(0);
    return 0;
}


// method to implement fg
int fg(char* args[])
{
    int count_args = 0;
    while(args[count_args] != NULL)
        count_args++;
    
    if (count_args != 2)
    {
        fprintf(stderr, "fg: Invalid number of arguments. SYNTAX: fg <pid>\n");
        handlePError(0);
        exitCode = -1;
        return 1;
    }

    // get pid of process
    pid_t pid = atoi(args[1]);
    if(pid < 0)
    {
        fprintf(stderr, "fg: Enter Positive integer for process ID, SYNTAX: fg <pid>\n");
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

    // get the data of the child process
    struct pData data = get_data_by_pid(pid);
    
    // error handling
    if(data.pid == -1)
    {
        fprintf(stderr, "fg: Process with PID: %d not in child pool\n", pid);
        handlePError(0);
        exitCode = -1;
        return 1;
    }

    // since we are moving the process to foreground
    // we remove from the child pool
    removeChildProcess(pid);

    // set the flag to 1 indicating that there is a foreground process running
    foreProcess = 1;

    // SIG_IGN - ignore the signal
    // SIG_DFL - default action
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);

    // set the process group id
    int pgid = getpgid(pid);

    // giving the process terminal control
    int retval = tcsetpgrp(STDIN, pgid);
    // error handling
    if(retval)
    {
        fprintf(stderr, "fg: Error in giving terminal control to process\n");
        signal(SIGTTOU, SIG_DFL);
        signal(SIGTTIN, SIG_DFL);
        handlePError(0);
        exitCode = -1;
        return 1;
    }

    // resuming the process
    retval = kill(pid, SIGCONT);
    // error handling
    if(retval)
    {
        fprintf(stderr, "fg: Error in resuming process\n");
        signal(SIGTTOU, SIG_DFL);
        signal(SIGTTIN, SIG_DFL);
        handlePError(0);
        exitCode = -1;
        return 1;
    }

    // wait for the process to complete
    // as it is a foreground process
    // handle all the control signals for this process
    // i.e. if I SIGSTP the process, it should be stopped and moved to the background
    int stat;
    int wait = waitpid(pid, &stat, WUNTRACED);
    if(wait > 0)
        if(WIFSTOPPED(stat))
            insertChildProcess(pid, data.pName, data.command, "Stopped");
        
    exitCode = WEXITSTATUS(stat) ? -1 : 1;

    int curr_pgid = getpgid(0);
    // give the terminal control back to the shell
    retval = tcsetpgrp(STDIN, curr_pgid);
    if(retval)
    {
        fprintf(stderr, "fg: Error in giving terminal control back to shell\n");
        signal(SIGTTOU, SIG_DFL);
        signal(SIGTTIN, SIG_DFL);
        handlePError(0);
        exitCode = -1;
        return 1;
    }

    // foreground process has exited
    // set it to 0
    foreProcess = 0;

    // return to default functioning of the signals
    signal(SIGTTOU, SIG_DFL);
    signal(SIGTTIN, SIG_DFL);

    return 0;
}
