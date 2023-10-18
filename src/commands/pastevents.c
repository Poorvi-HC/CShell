// this file has the implementtation for pastevents
// replica of the "history" function in linux machines

// custom header files
#include "pastevents.h"

// variables defined elsewhere
extern char HISTORY_ARRAY[MAX_STORAGE][MAX_COMMAND_LEN + 1];
extern char history_index;
extern char INVOC_LOC[];


// loads the command into the history_array
// returns 0 if unsucessful and returns 1 else
int loadPastEvents(char* COMMAND)
{
    // if the COMMAND has the term "pastevents" exit function and do not load into file/array
    if (strstr(COMMAND, "pastevents") != NULL)
        return 0;
    
    char temp[MAX_COMMAND_LEN + 1];
    strcpy(temp, COMMAND);

    // Trim trailing whitespace, including newline characters
    int len = strlen(temp);
    while (len > 0 && (temp[len - 1] == ' ' || temp[len - 1] == '\n')) {
        temp[len - 1] = '\0';
        len--;
    }
    
    if (history_index < MAX_STORAGE)
    {
        // compare if the last command is the same as the curr command
        if (history_index > 0 && !strcmp(temp, HISTORY_ARRAY[history_index - 1]))
            return 0;
        else
        {
            // store in history_index
            strcpy(HISTORY_ARRAY[history_index], temp);
            history_index++;
            savePastEvents();
            return 1;
        }
    }
    else
    {
        // compare if the last command is the same as the curr command
        if (history_index > 0 && !strcmp(temp, HISTORY_ARRAY[history_index - 1]))
            return 0;
        else
        {
            // move all the contents in the array uphill (shift contents south)
            move_uphill();
            strcpy(HISTORY_ARRAY[history_index], temp);
            history_index++;
            savePastEvents();
            return 1;
        }
    }
}

// move all contents in the array into a history.txt file
void savePastEvents()
{
    // maintain current working directory
    char CWD[PATH_MAX + 1];
    getcwd(CWD, PATH_MAX + 1);

    // render the invocation directory
    chdir(INVOC_LOC);
    // create and open "history.txt" file
    FILE* historyFile = fopen("history.txt", "w");
    for(int i = 0; i < history_index;i++)
    {
        fprintf(historyFile, "%s\n", HISTORY_ARRAY[i]);
    }
    fclose(historyFile);
    chdir(CWD);
}

// move all contents in the array uphill
void move_uphill()
{
    // if the history_index is exceeding 15 
    // delete the initial commands 
    // move all the commands uphill

    for (int i = 1;i<history_index;i++)
    {
        strcpy(HISTORY_ARRAY[i - 1], HISTORY_ARRAY[i]);
    }
    history_index--;
}

// run the "pastevents" command
int runPastEvents(char* args[])
{
    if (args[1] == NULL)
    {
        // if there exists no args[1] then run simple pastevents()
        if(pastevents(args)){
            handlePError(0);
        }
    }
    else if (!strcmp(args[1], "purge"))
    {
        if(pastEventsPurge(args)){
            handlePError(0);
        }
    }
    else if (!strcmp(args[1], "execute"))
    {
        if(pastEventsExec(args)){
            handlePError(0);
        }
    }
    else
    {
        errno = EINVAL;
        handlePError(0);
    }
    return 0;
}

// method to run the pastevents command
int pastevents(char* args[])
{
    // print the HISTORY_ARRAY
    for (int i = 0;i<history_index;i++)
    {
        printf("%s\n", HISTORY_ARRAY[i]);
    }
    return 0;
}

// method to run the pastevents purge command
int pastEventsPurge(char *args[])
{
    // if there exists no args[2] then run simple pastevents purge
    if (args[2] == NULL)
    {
        // delete all the contents of the history.txt file
        // and clear the HISTORY_ARRAY
        for (int i = 0;i<history_index;i++)
        {
            strcpy(HISTORY_ARRAY[i], "");
        }
        history_index = 0;
        savePastEvents();
        return 0;
    }
    else
    {
        errno = EINVAL;
        return -1;
    }
    return 0;
}

// method to run the pastevents execute command
int pastEventsExec(char* args[])
{
    // args[2] is the index of the command to be executed
    // if args[2] is not given, then return error
    
    if (args[2] == NULL)
    {
        errno = EINVAL;
        return -1;
    }
    else
    {
        // convert args[2] to int
        int index = atoi(args[2]);
        if (index > history_index)
        {
            errno = EINVAL;
            return -1;
        }
        else
        {
            // process the command which eventually runs it
            // add newline to the end of the command
            char temp[MAX_COMMAND_LEN + 1];
            strcpy(temp, HISTORY_ARRAY[history_index - index]);
            strcat(temp, "\n");
            loadPastEvents(temp);
            processInputString(temp);
        }
    }
    return 0;
}

// read the history.txt file and load the contents into the HISTORY_ARRAY
void readPastEvents()
{
    // if history.txt file does not exist create a empty file
    FILE* historyFile = fopen("history.txt", "a");
    fclose(historyFile);
    // reading from file into array
    historyFile = fopen("history.txt", "r");
    char string[MAX_COMMAND_LEN + 1];

    while(fgets(string, MAX_INPUT_LEN + 1, historyFile) != NULL){
        strcpy(HISTORY_ARRAY[history_index], string);
        HISTORY_ARRAY[history_index][strlen(HISTORY_ARRAY[history_index]) - 1] = '\0';
        history_index++;
    }
    fclose(historyFile);
}