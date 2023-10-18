// main C file for the cshell project

// custom header files
#include "definitions.h"
#include "./commands/echo.h"
#include "utilities.h"
#include "./commands/proclore.h"
#include "./commands/warp.h"
#include "./commands/pastevents.h"
#include "./commands/sig.h"

char SHELL_NAME[] = "CShell";
char GREET[100] = "Welcome to CShell!";

// imp global variables
// LOGIN_NAME_MAX, HOST_NAME_MAX, PATH_MAX are defined in limits.h
char PString[LOGIN_NAME_MAX + HOST_NAME_MAX + PATH_MAX + 5];
char INVOC_LOC[PATH_MAX + 1]; // location of invocation of the shell
char PREV_LOC[PATH_MAX + 1]; 
char HISTORY_ARRAY[MAX_STORAGE][MAX_COMMAND_LEN + 1];
char history_index = 0;
int recursive_call = 0;
char foreProcess = 0;
char fgFlag = 0;
char pinged = 0;


int main()
{
    // initialize the shell
    // handle and install the handlers as soon as the shell runs
    initIntHandlers();
    initStpHandlers();
    initChildHandlers();

    // Initialzing child pool with empty process info instances
    initChildren();

    char INPUT_STRING[MAX_COMMAND_LEN + 1];
    clear();
    int ps_state = generatePString(1, PString, INVOC_LOC);
    psError(ps_state);

    // Set flag to 0 indicating there are no foreground processes running on top of the shell
    foreProcess = 0;

    // read History file contents into HISTORY_ARRAY when shell rendered
    readPastEvents();

    // Entering the infinite loop which 
    // 1. prints the prompt string
    // 2. gets the command 
    // 3. executes it

    while(1)
    {
        if(!pinged){
            printf("%s", PString);
            pinged = 0;
        }
        // If Ctrl-D is observed, exit the shell
        if (!fgets(INPUT_STRING, MAX_COMMAND_LEN, stdin)){
            break;
        }
        else if (strcmp(INPUT_STRING, "\n")){
            // store the input string in HISTORY_ARRAY
            loadPastEvents(INPUT_STRING);
            processInputString(INPUT_STRING);
        }
        else{
            printf("\n");
        }
        generatePString(0, PString, INVOC_LOC);
    }

    exit(0);

}





