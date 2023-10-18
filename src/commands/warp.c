// implements the warp command
// identical to "cd" command in linux

// custom header files
#include "warp.h"

// Variables Declared elsewhere
extern char TEMP_BUFFER[]; // buffer for printing out errors or any string/data
extern char INVOC_LOC[];
extern char PREV_LOC[];
extern char SHELL_NAME[];
extern int exitCode;

// method to implement warp command
int warp(char *arg)
{
    // copy argument into a new buffer
    int arg_len = strlen(arg);
    char *path = (char *)malloc(arg_len + 1);
    strcpy(path, arg);

    // get username
    struct passwd *U_INFO = NULL;
    U_INFO = getpwuid(getuid());
    char UNAME[LOGIN_NAME_MAX];
    strcpy(UNAME, U_INFO->pw_name);

    // path to be printed
    char PATH[PATH_MAX + PATH_MAX + 1];
    char code;
    char temp[PATH_MAX + 1];
    char CWD[PATH_MAX + 1];

    // temp now has stored the previous working directory info
    strcpy(temp, PREV_LOC);
    // now store the current working directory as previous directory
    getcwd(PREV_LOC, PATH_MAX);
    getcwd(CWD, PATH_MAX);

    if (path == NULL || !strcmp(path, "~"))
    {
        code = chdir(INVOC_LOC);
        if (code == 0)
        {
            sprintf(PATH, "/home/%s", UNAME);
            printf("%s\n", PATH);
        }
    }
    else if (!strcmp(path, "-"))
    {
        // handles cases given in relative paths
        // move to previous location

        strcpy(TEMP_BUFFER, temp);
        shortenPathSubDir(INVOC_LOC, TEMP_BUFFER);

        // As TEMP_BUFFER is not modifiable, we create a new buffer
        char *temp2 = (char *)malloc(512);
        strcpy(temp2, TEMP_BUFFER);

        // move/change directories
        code = chdir(temp);
        if (!strcmp(temp2, "~"))
        {
            // code = chdir(temp2);
            if (code == 0)
            {
                sprintf(PATH, "/home/%s", UNAME);
                printf("%s\n", PATH);
            }
        }
        else
        {
            // code = chdir(INVOC_LOC);
            char *temp_copy = temp2 + 2; // move pointer 2 bytes down to remove the ~/
            // code = chdir(temp_copy);
            // create PATH variable
            sprintf(PATH, "/home/%s/%s", UNAME, temp_copy);
            printf("%s\n", PATH);
        }
        free(temp2);
    }
    else if (!strcmp(path, ".."))
    {
        // handles cases given in relative paths
        // move to parent location

        char parent_path[PATH_MAX + 1];

        // get parent path
        const char *last_slash = strrchr(CWD, '/');

        // if last_slash is null, then we are in root directory
        if (last_slash == NULL)
        {
            code = chdir(INVOC_LOC);
            if (code == 0)
            {
                sprintf(PATH, "/home/%s", UNAME);
                printf("%s\n", PATH);
            }
        }
        else
        {
            // copy the parent path into parent_path
            strncpy(parent_path, CWD, strlen(CWD) - strlen(last_slash));
            parent_path[strlen(CWD) - strlen(last_slash)] = '\0';

            // move to parent path
            code = chdir(parent_path);

            // create PATH variable
            if (code == 0)
            {
                shortenPathSubDir(INVOC_LOC, parent_path); // shorten parent_path
                char *parent_path_copy = &parent_path[0] + 2;

                sprintf(PATH, "/home/%s/%s", UNAME, parent_path_copy);
                printf("%s\n", PATH);
            }
        }
    }
    else if (!strcmp(path, "."))
    {
        // remains in the Current working directory
        code = chdir(CWD);
        shortenPathSubDir(INVOC_LOC, CWD);
        printf("%s\n", CWD);

        if (!strcmp(CWD, "~"))
        {
            if (code == 0)
            {
                sprintf(PATH, "/home/%s", UNAME);
                printf("%s\n", PATH);
            }
        }
        else
        {
            char *CWD_copy = &CWD[0] + 2;
            if (code == 0)
            {
                snprintf(PATH, sizeof(PATH), "/home/%s/%s", UNAME, CWD_copy);
                printf("%s\n", PATH);
            }
        }
    }
    else if (path[0] == '~')
    {
        // handles cases given in absolute paths
        // eg: ~/Desktop
        code = chdir(INVOC_LOC);
        path += 2; // move pointer 2 bytes down to remove the ~/
        code = chdir(path);
        if (code == 0)
        {
            sprintf(PATH, "/home/%s/%s", UNAME, path);
            printf("%s\n", PATH);
        }
    }
    else
    {
        shortenPathSubDir(INVOC_LOC, CWD);

        if (!strcmp(CWD, "~"))
        {
            code = chdir(path);
            if (code == 0)
            {
                sprintf(PATH, "/home/%s/%s", UNAME, path);
                printf("%s\n", PATH);
            }
        }
        else
        {
            char *CWD_copy = &CWD[0] + 2;

            code = chdir(path);
            if (code == 0)
            {
                snprintf(PATH, sizeof(PATH), "/home/%s/%s/%s", UNAME, CWD_copy, path);
                printf("%s\n", PATH);
            }
        }
    }

    if (code) // reaches an invalid location
    {
        exitCode = -1;
        strcpy(PREV_LOC, temp);
    }
    return code;
}

// method to run the warp command
int runWarp(char *args[])
{
    // if no arguments are given, go to home directory
    char *str = "~";
    if (args[1] == NULL)
    {
        if (warp(str))
            handlePError(0);
        return 0;
    }

    // if arguments are given, go to each of them
    for (int i = 1; args[i] != NULL; i++)
    {
        if (warp(args[i]))
            handlePError(0);
    }
    return 0;
}
