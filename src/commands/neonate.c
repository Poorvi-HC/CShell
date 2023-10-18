// this file contains the implementation of neaonate
// flag = -n
// command structure:
//     neonate -n <time_args>
// command prints pids of the most recently created process on the system
// this pids are printed every time_args secs until 'x' key is pressed

// include custom header files
#include "neonate.h"

// variables defined elsewhere
extern char exitCode;

struct termios orig_termios;

void die(const char *s)
{
    perror(s);
    exit(1);
}

void disableRawMode()
{
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
        die("tcsetattr");
}

void enableRawMode()
{
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1)
        die("tcgetattr");
    atexit(disableRawMode);
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
        die("tcsetattr");
}

// method for keyboard hit detection
int kbhit() {
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}


// method to run the neonate command
int runNeonate(char *args[])
{
    if (neonate(args))
        handlePError(0);
    return 0;
}

// neonate function
int neonate(char *args[])
{
    int count_args = 0;
    while (args[count_args] != NULL)
    {
        count_args++;
    }

    // invalid number of arguments
    if (count_args != 3)
    {
        fprintf(stderr, "neonate: Invalid number of arguments. SYNTAX: neonate -n <time_args>\n");
        handlePError(0);
        exitCode = -1;
        return 1;
    }

    // check if the flag exists after neonate
    if (strcmp(args[1], "-n"))
    {
        fprintf(stderr, "neonate: Invalid flag. SYNTAX: neonate -n <time_args>\n");
        handlePError(0);
        exitCode = -1;
        return 1;
    }

    // get time_args
    // check time_args is a positive integer > 0
    int time_args = atoi(args[2]);
    if (time_args < 0)
    {
        fprintf(stderr, "neonate: Enter Positive integer for time_args, SYNTAX: neonate -n <time_args>\n");
        handlePError(0);
        exitCode = -1;
        return 1;
    }

    // to check if time_args is not a float
    // check if each element in timeargs is a digit
    for(int i = 0;i< strlen(args[2]);i++){
        if(!isdigit(args[2][i])){
            fprintf(stderr, "neonate: Enter Positive integer, not floats for time_args, SYNTAX: neonate -n <time_args>\n");
            handlePError(0);
            exitCode = -1;
            return 1;
        }
    }

    // enable rawMode
    enableRawMode();

    // print PIDs
    while(1)
    {
        if(kbhit()){
            char c = getchar();
            if (c == 'x'){
                break;
            }
        }

        // get last word from /proc/loadavg
        FILE *fp = fopen("/proc/loadavg", "r");
        if (fp == NULL) {
            printf("Could not open file\n");
            return 1;
        }

        char line[100];
        char prev_word[100];
        char* last_word = NULL;

        fgets(line, sizeof(line), fp);
        last_word = strtok(line, " ");
        while (last_word != NULL) {
            strcpy(prev_word,last_word);
            last_word = strtok(NULL, " ");
        }

        printf("%s", prev_word);
        fclose(fp);


        // sleep for time_args secs
        sleep(time_args);
    }

    disableRawMode();

    return 0;
}
