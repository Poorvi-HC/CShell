// utilities.c -> contains all the shell related utility functions
// utility functions are those which are not imp enough to considered functionalities

// Custom header files
#include "definitions.h"
#include "utilities.h"
// #include "./commands/echo.h"
#include "./commands/warp.h"
#include "./commands/peek.h"
#include "./commands/proclore.h"
#include "./commands/pastevents.h"
#include "./commands/seek.h"
#include "./commands/system.h"
#include "./commands/activities.h"
#include "./commands/ping.h"
#include "./commands/sig.h"
#include "./commands/fg.h"
#include "./commands/bg.h"
#include "./commands/neonate.h"
#include "./commands/iman.h"

// Global Variables declared elsewhere
extern char INVOC_LOC[];
extern char PREV_LOC[];
extern char SHELL_NAME[];
extern char GREET[];
extern char PString[];
extern char HISTORY_ARRAY[MAX_STORAGE][MAX_COMMAND_LEN + 1];
extern char history_index;
extern char foreProcess;
extern char pinged;

// pool of child processes
struct pData childArray[MAX_CHILD_COUNT];

// current process
struct pData currProcess;

int exitCode;
// number of active children process
int childCount = 0;
double fgExecutionTime = 0;
char chld_argument[1024];

// Temporary buffer to print arbitrary messages to the terminal
char TEMP_BUFFER[512];

// method to clear the terminal
void clear()
{
    printf("\e[1;1H\e[2J");
}

// method to generate the PString / prompt string
// arguments: init ((intial string? 0 / 1), PString (pointer to the string where we are storing it), INVOC_LOC (Shell invocation location)))
int generatePString(char init, char *PString, char *INVOC_LOC)
{
    // get username
    // error RV = -1
    struct passwd *U_INFO = NULL;
    uid_t U_ID = getuid();
    U_INFO = getpwuid(U_ID);
    char Username[LOGIN_NAME_MAX + 1];
    strcpy(Username, U_INFO->pw_name);
    if (Username == NULL)
        return -1;

    // get the system name
    // error RV = -2
    char SYS_NAME[HOST_NAME_MAX + 1];
    int sysname_rv;
    sysname_rv = gethostname(SYS_NAME, HOST_NAME_MAX);
    if (sysname_rv == -1)
        return -2;

    // store current directory as home directory
    // error RV = -3
    if (init)
    {
        // this is the first call of generate_PString
        if (!getcwd(INVOC_LOC, PATH_MAX))
            return -3;
        strcpy(PREV_LOC, INVOC_LOC);
        exitCode = 0;
    }

    // get curr directory
    // error RV = -4
    char PWD[PATH_MAX + 1];
    if (!getcwd(PWD, PATH_MAX))
        return -4;

    // if curr directory is a sub directory of the invocation location, shorten it
    shortenPathSubDir(INVOC_LOC, PWD);

    // if curr directory is the same as the invocation location, replace it with ~
    sprintf(PString, "\033[1;36m<\033[1;33m%s@%s:\033[1;34m%s\033[1;36m", Username, SYS_NAME, PWD);
    // Calculate execution time if previous foreground process took more than 2 seconds
    if (fgExecutionTime > 2)
    {
        // round down exec time
        int execTime = (int)fgExecutionTime;

        // append to existing PString
        sprintf(PString, "%s %s : %ds>\033[0m ", PString, chld_argument, execTime);
        fgExecutionTime = 0;
    }
    else
    {
        // append ">" to PString
        strcat(PString, ">\033[0m ");
    }
    return 0;
}

// method to shorten the path if it a subdirectory of the INVOC_LOC
// arguments: INVOC_LOC (Invocation location), PWD (current directory)
void shortenPathSubDir(char *INVOC_LOC, char *PWD)
{
    int invocation_len = strlen(INVOC_LOC);
    int curr_dir_len = strlen(PWD);

    // If INVOC_LOC is a substring from the beginning of PWD
    // then PWD is a subdirectory of INVOC_LOC
    if (!strncmp(INVOC_LOC, PWD, invocation_len))
    {
        // replace the INVOC_LOC with ~
        char temp[PATH_MAX + 1] = "~";
        // shift the rest of the string to the left
        for (int i = 1; i < curr_dir_len - invocation_len + 1; i++)
            temp[i] = PWD[i + invocation_len - 1];

        strcpy(PWD, temp);
    }
}

// Method to print the perror messages and exit if necessary
void handlePError(int quit)
{
    sprintf(TEMP_BUFFER, "%s Error", SHELL_NAME);
    perror(TEMP_BUFFER);
    if (quit)
        exit(1);
}

// Method to handle all prompt string generation issues
void psError(int psState)
{
    if (psState == 0)
        printf("%s \n", GREET);
    else if (psState == -1)
        fprintf(stderr, "Couldn't retrieve username. Unable to launch shell.\n");
    else if (psState == -2)
        fprintf(stderr, "Couldn't retrieve system name. Unable to launch shell.\n");
    else if (psState == -3)
        fprintf(stderr, "Couldn't retrieve directory name. Unable to launch shell.\n");
    else if (psState == -4)
        fprintf(stderr, "Couldn't retrieve directory name. Terminating Process.\n");
    if (psState)
        handlePError(1);
}

// Utility function which processes the inputted string
void processInputString(char *INPUT_STRING)
{
    int len = strlen(INPUT_STRING);
    int no_commands = 0;

    // replace tabs with spaces and newline with ;
    for (int i = 0; i < len; i++)
    {
        if (INPUT_STRING[i] == '\t')
            INPUT_STRING[i] = ' ';
        if (INPUT_STRING[i] == '\n')
            INPUT_STRING[i] = ';';
        if (INPUT_STRING[i] == ';')
            no_commands++;
    }

    // create a pointer for each command storage
    char *COMMANDS[no_commands + 1] = {NULL};

    int index = 0;
    COMMANDS[index] = strtok(INPUT_STRING, ";");
    while (COMMANDS[index] != NULL && index < no_commands)
    {
        index++;
        COMMANDS[index] = strtok(NULL, ";");
    }

    // process each command
    for (int i = 0; i < no_commands; i++)
        process_amp(COMMANDS[i]);
}

// this method clears out the additional spaces in each command.
// once processed it will be streamlined to execute if it is a non empty string
void processCommand(char *COMMAND_STRING)
{
    char *token;
    char TOKEN[PATH_MAX + 1];

    // String which stores the final cleaned command
    char CLEANED_COMMAND[MAX_COMMAND_LEN + 1] = "";

    // split the command string into arguments
    token = strtok(COMMAND_STRING, " ");
    while (token)
    {
        sprintf(TOKEN, "%s ", token);
        strcat(CLEANED_COMMAND, TOKEN);
        token = strtok(NULL, " ");
    }

    // remove trailing space
    size_t length = strlen(CLEANED_COMMAND);
    if (length > 0 && CLEANED_COMMAND[length - 1] == ' ')
        CLEANED_COMMAND[strlen(CLEANED_COMMAND) - 1] = '\0';

    if (strlen(CLEANED_COMMAND))
        managePipes(CLEANED_COMMAND);
}

// method to process &
void process_amp(char *COMMAND_STRING)
{
    int len = strlen(COMMAND_STRING);
    int no_amps = 0;

    for (int i = 0; i < len; i++)
    {
        if (COMMAND_STRING[i] == '&')
            no_amps++;
    }

    int no_commands = (COMMAND_STRING[strlen(COMMAND_STRING) - 1] == '&') ? no_amps : no_amps + 1;
    // Check if the command ends with "&"
    // int hasAmpersand = (COMMAND_STRING[strlen(text) - 1] == '&');

    // create a pointer for each command storage
    char *COMMANDS[no_commands] = {NULL};

    int index = 0;
    COMMANDS[index] = strtok(COMMAND_STRING, "&");
    while (COMMANDS[index] != NULL && index < no_commands)
    {
        index++;
        COMMANDS[index] = strtok(NULL, "&");
    }

    char text[MAX_COMMAND_LEN + 1];
    // process each command

    // If no_amps < no_commands, attach "&" to all commands except the last one
    // If no_amps == no_commands, attach "&" to all commands
    for (int i = 0; i < no_commands; i++)
    {
        strcpy(text, COMMANDS[i]);
        if (no_amps < no_commands)
        {
            if (i != no_commands - 1)
            {
                strcat(text, "&");
            }
        }
        else if (no_amps == no_commands)
        {
            strcat(text, "&");
        }
        processCommand(text);
    }
}

// method to handle the pipes
void managePipes(char *CLEAN_COMMAND)
{
    // Count the number of pipe-separated commands
    int count = 0;
    for (int i = 0; i < strlen(CLEAN_COMMAND); i++)
    {
        if (CLEAN_COMMAND[i] == '|')
            count++;
    }
    count++;

    // If there are no pipes, execute the command
    if (count == 1)
        execCommand(CLEAN_COMMAND);

    // if there are pipes, handle them
    else
    {
        // maintain a pointer for each piped command
        // how to handle pipes
        // 1. if N piped commands present, create N - 1 child processes, each handling one command
        // 2. pass input of one command to the next command

        // tokenize the input into pipe seperated commands
        char *COMMANDS[count];
        const char *delimiter = "|";
        int index = 0;

        char *token = strtok(CLEAN_COMMAND, delimiter);

        while (token != NULL)
        {
            // trimming leading and trailing spaces from token
            int start = 0, end = strlen(token) - 1;

            while (isspace(token[start]))
                start++;
            while (end > start && isspace(token[end]))
                end++;

            int trimmed_len = end - start + 1;

            COMMANDS[index] = (char *)malloc(sizeof(char) * trimmed_len + 1);
            strncpy(COMMANDS[index], token + start, trimmed_len);
            COMMANDS[index][trimmed_len] = '\0';

            index++;
            token = strtok(NULL, delimiter);
        }
        
        if(index != count)
        {
            fprintf(stderr, "Invalid use of pipe\n");
            handlePError(0);
            exitCode = -1;
            return;
        }

        // maintain number of pipes and
        // array of file descriptors from each pipe function call
        int pipes = count - 1;
        int fds[2 * pipes];

        // create pipes
        for (int i = 0; i < 2 * pipes; i += 2)
        {
            if (pipe(fds + i))
            {
                fprintf(stderr, "Error creating pipe\n");
                handlePError(0);
                exitCode = -1;
                return;
            }
        }
        int pid;

        // create child processes for each command
        for (int i = 0; i < count; i++)
        {
            // create a child process
            pid = fork();

            // if fork fails, print error and exit
            if (pid < 0)
            {
                fprintf(stderr, "Error creating child process\n");
                handlePError(0);
                exitCode = -1;
                return;
            }

            // if child process
            if (pid == 0)
            {
                // if not first command
                if (i != 0)
                {
                    // redirect STDIN to the read end of the previous pipe
                    if (dup2(fds[2 * (i - 1)], STDIN) == -1)
                    {
                        fprintf(stderr, "Error duplicating STDIN\n");
                        handlePError(0);
                        exitCode = -1;
                        return;
                    }
                }

                // if not last command
                if (i != count - 1)
                {
                    // redirect STDOUT to the write end of the current pipe
                    if (dup2(fds[2 * i + 1], STDOUT) == -1)
                    {
                        fprintf(stderr, "Error duplicating STDOUT\n");
                        handlePError(0);
                        exitCode = -1;
                        return;
                    }
                }

                // close all file descriptors
                for (int j = 0; j < 2 * pipes; j++)
                    close(fds[j]);

                // execute the command
                execCommand(COMMANDS[i]);
                exit(exitCode);
            }
        }

        // close all file descriptors
        for (int i = 0; i < 2 * pipes; i++)
            close(fds[i]);

        // wait for all child processes to terminate
        // collect thier exit statuses
        for (int i = 0; i < count - 1; i++)
            wait(NULL);

        wait(&exitCode);
        exitCode = WEXITSTATUS(exitCode);

        // Free allocated memory when done
        for (int i = 0; i < index; i++)
        {
            free(COMMANDS[i]);
        }
    }
}

void execCommand(char *COMMAND)
{
    int len = strlen(COMMAND);

    // break command into arguments
    char temp[MAX_INPUT_LEN + 2]; // copy contents of command into temp
    strcpy(temp, COMMAND);
    char *token = strtok(temp, " ");

    // store the arguments in an array called args
    char *args[MAX_ARGUMENTS] = {NULL};
    args[0] = token;
    int index = 1;
    while (token != NULL && index < MAX_ARGUMENTS)
    {
        token = strtok(NULL, " ");
        args[index] = token;
        index++;
    }

    // handles I/O redirection
    // check if redirection is even needed
    // also store the location of the file in the command/ arguments array
    int read = 0, write = 0, append = 0;
    // flags for reading, writing or appending to or from a file
    int input = -1, output = -1;
    // stores the position of the filename in the arguments array

    for (int i = 0; i < index; i++)
    {
        if (args[i] == NULL)
            break;
        if (!strcmp(args[i], "<"))
        {
            read = 1;
            input = i + 1;
        }
        else if (!strcmp(args[i], ">"))
        {
            write = 1;
            output = i + 1;
        }
        else if (!strcmp(args[i], ">>"))
        {
            append = 1;
            output = i + 1;
        }
    }

    // if redirection is needed
    // obtain backup for STDIN and STDOUT
    int inpBack, outBack;
    // file descriptors
    int inpFD, outFD;

    // variable to store original STDIN
    int original_stdin = dup(STDIN_FILENO);

    if (read)
    {
        // printf("read \n");
        // backup STDIN into inpBack
        inpBack = dup(STDIN);
        // open the file in read only mode
        inpFD = open(args[input], O_RDONLY);
        // if file open fails, print error and exit
        if (inpFD < 0)
        {
            close(inpBack);
            fprintf(stderr, "Error opening file %s\n", args[input]);
            handlePError(0);
            exitCode = -1;
            return;
        }
        // redirect STDIN to the file
        // handle error in duplication of STDIN
        if (dup2(inpFD, STDIN) == -1)
        {
            close(inpBack);
            fprintf(stderr, "Error duplicating STDIN\n");
            handlePError(0);
            exitCode = -1;
            return;
        }
    }

    // Variables to save original STDOUT
    int original_stdout = dup(STDOUT_FILENO);

    if (write || append)
    {
        // printf("write or append \n");
        // backup STDOUT into outBack
        outBack = dup(STDOUT);
        if (write)
        {
            // if we want to write into an input file
            // O_CREAT -> create file if it doesnt exist
            // O_WRONLY -> write only
            // O_TRUNC -> truncate the file
            outFD = open(args[output], O_CREAT | O_WRONLY | O_TRUNC, 0644);
        }
        if (append)
        {
            // if we want to append into an input file
            // O_APPEND -> append to the file
            outFD = open(args[output], O_CREAT | O_WRONLY | O_APPEND, 0644);
        }

        // if file open fails, print error
        if (outFD < 0)
        {
            close(outBack);
            fprintf(stderr, "Error opening file %s\n", args[output]);
            handlePError(0);
            exitCode = -1;
            return;
        }
        // redirect STDOUT to the file
        // if error print it
        if (dup2(outFD, STDOUT) == -1)
        {
            close(outBack);
            fprintf(stderr, "Error duplicating STDOUT\n");
            handlePError(0);
            exitCode = -1;
            return;
        }
    }

    // execute the commands
    if (!strcmp(args[0], "exit") || !strcmp(args[0], "quit"))
        exit(0);
    else if (!strcmp(args[0], "clear"))
    {
        clear();
    }
    else if (!strcmp(args[0], "warp"))
    {
        if (write || append)
            args[output - 1] = NULL;
        if (read)
            args[input - 1] = NULL;
        runWarp(args);
    }
    else if (!strcmp(args[0], "peek"))
    {
        if (write || append)
            args[output - 1] = NULL;
        if (read)
            args[input - 1] = NULL;
        runPeek(args);
    }
    else if (!strcmp(args[0], "proclore"))
    {
        runProclore(args);
    }
    else if (!strcmp(args[0], "pastevents"))
    {
        if (write || append)
            args[output - 1] = NULL;
        if (read)
            args[input - 1] = NULL;
        runPastEvents(args);
    }
    else if (!strcmp(args[0], "seek"))
    {
        if (write || append)
            args[output - 1] = NULL;
        if (read)
            args[input - 1] = NULL;
        runSeek(args);
    }
    else if (!strcmp(args[0], "activities"))
    {
        if (write || append)
            args[output - 1] = NULL;
        if (read)
            args[input - 1] = NULL;
        runActivities(args);
    }
    else if(!strcmp(args[0], "ping"))
    {
        int retval = runPing(args);
        if(retval == 0)
            pinged = 1;
    }
    else if(!strcmp(args[0], "fg"))
    {
        run_fg(args);
    }
    else if(!strcmp(args[0], "bg"))
    {
        run_bg(args);
    }
    else if(!strcmp(args[0], "neonate"))
    {
        runNeonate(args);
    }
    else if(!strcmp(args[0], "iMan"))
    {
        if (write || append)
            args[output - 1] = NULL;
        if (read)
            args[input - 1] = NULL;
        run_iMan(args);
    }
    else
    {
        // if the command is not a built in command
        // read and write handle:
        if (write || append)
            args[output - 1] = NULL;
        if (read)
            args[input - 1] = NULL;

        // check if its a background process or not
        bool isBackground = (COMMAND[strlen(COMMAND) - 1] == '&');

        if (isBackground)
        {
            COMMAND[strlen(COMMAND) - 1] = '\0';
            if (background(args, COMMAND))
                handlePError(0);
        }
        else
        {
            fgExecutionTime = 0;
            if (foreground(args, COMMAND))
                handlePError(0);
        }
    }

    if (read)
        close(inpFD);
    if (write || append)
        close(outFD);

    // Restore original STDOUT
    dup2(original_stdout, STDOUT_FILENO);
    // Restore original STDIN
    dup2(original_stdin, STDIN_FILENO);
}

long long int max(long long int a, long long int b)
{
    return a > b ? a : b;
}

// Utility function to get the number of digits in a number
int digitCount(long long x)
{
    int dig;
    for (dig = 1; x /= 10; dig++) {}
    return dig;
}

// insert child into the pool
// provided its name and pid
int insertChildProcess(int pid, char *processName, char *command, char* status)
{
    // if the children pool is full
    if (childCount == MAX_CHILD_COUNT)
    {
        fprintf(stderr, "No more space available\n");
        exitCode = -1;
        return -1;
    }

    // insert the child into the pool
    childArray[childCount].pid = pid;
    strcpy(childArray[childCount].pName, processName);
    strcpy(childArray[childCount].command, command);
    strcpy(childArray[childCount].status, status);
    childCount++;
    return 0;
}

// function to initialize the children pool to empty processes
void initChildren()
{
    for (int i = 0; i < MAX_CHILD_COUNT; i++)
    {
        childArray[i].pid = -1;
        strcpy(childArray[i].pName, "");
        strcpy(childArray[i].command, "");
        strcpy(childArray[i].status, "");
    }
}

// method to remove a child process from the pool
void removeChildProcess(pid_t pid)
{
    char found = 0;
    // If a child process that is in the middle of the array is killed, the processes
    // ahead of it are moved back one space to keep them contiguous
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
                strcpy(childArray[i].command, childArray[i + 1].command);
                strcpy(childArray[i].status, childArray[i + 1].status);
            }
        }
    }
    if(found)
        childCount--;
}

// initialize signal handlers
// new and interesting concept and code
void initChildHandlers()
{
    // SIGCHLD
    // create a signal handler for SIGCHLD
    // create a struct sigaction instance
    struct sigaction chld_action;
    memset(&chld_action, 0, sizeof(chld_action));

    // handler function - sigchldHandler
    chld_action.sa_handler = sigchldHandler;

    // Signal handled
    // continue halted sys calls
    chld_action.sa_flags = SA_RESTART;

    sigemptyset(&chld_action.sa_mask);
    sigaction(SIGCHLD, &chld_action, NULL);
}

void initIntHandlers()
{
    struct sigaction int_action;
    memset(&int_action, 0, sizeof(int_action));
    int_action.sa_handler = sigintHandler;
    int_action.sa_flags = SA_RESTART;
    sigemptyset(&int_action.sa_mask);
    sigaction(SIGINT, &int_action, NULL);
}

void initStpHandlers()
{
    struct sigaction stp_action;
    memset(&stp_action, 0, sizeof(stp_action));
    stp_action.sa_handler = sigstpHandler;
    stp_action.sa_flags = SA_RESTART;
    sigemptyset(&stp_action.sa_mask);
    sigaction(SIGTSTP, &stp_action, NULL);
}

void initQuitHandlers()
{
    struct sigaction quit_action;
    memset(&quit_action, 0, sizeof(quit_action));
    quit_action.sa_handler = sigquitHandler;
    quit_action.sa_flags = SA_RESTART;
    sigemptyset(&quit_action.sa_mask);
    sigaction(SIGQUIT, &quit_action, NULL);
}

struct pData get_data_by_pid(pid_t pid)
{
    for (int i = 0; i < childCount; i++)
    {
        if (childArray[i].pid == pid)
            return childArray[i];
    }
    struct pData temp;
    temp.pid = -1;
    return temp;
}



