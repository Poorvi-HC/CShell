// this file contains the implementation for seek function

// include custom header files
#include "seek.h"

// global Variables
char DIRECTORY[PATH_MAX + 1];
char KEYWORD[PATH_MAX + 1];
extern char INVOC_LOC[];
extern char PREV_LOC[];
// used by the -e flag as this will rewrite in all other casess
char PATH_STORAGE[PATH_MAX + 1];

// global variable initialized elsewhere
extern int recursive_call;
int no_file_match = 0;
int no_dir_match = 0;

// runSeek function
// method to run seek command
int runSeek(char *args[])
{
    if (seek(args))
        handlePError(0);
    return 0;
}

// function for locating the file in the given directory
// and print their paths

void recurse_for_File(char file[PATH_MAX], char dir[PATH_MAX], char string[PATH_MAX + 1], int FlagF, int FlagD)
{
    recursive_call++;
    // printf("file: %s\n", file);
    char path[PATH_MAX];

    struct dirent *dirContent;
    DIR *dp = opendir(dir);
    if (dp == NULL)
    {
        errno = EACCES;
        return;
    }

    while ((dirContent = readdir(dp)) != NULL)
    {
        // skip .. or . i.e. parent or current directory
        // including them will lead to an infinite loop
        if (strcmp(dirContent->d_name, "..") == 0 || strcmp(dirContent->d_name, ".") == 0)
            continue;
        sprintf(path, "%s/%s", dir, dirContent->d_name);
        char new_string[PATH_MAX];
        sprintf(new_string, "%s/%s", string, dirContent->d_name);
        char check[PATH_MAX];
        strcpy(check, dirContent->d_name);

        // check if file is present in the directory
        if (strstr(new_string, file) != NULL && strstr(check, file) != NULL)
        {

            // printf("%s\n", new_string);
            // if flagF is present only KEYWORDs are printed
            if (FlagF == 1)
            {
                if (dirContent->d_type != 4)
                {
                    // sprintf(new_string, "%s%s%s", ANSI_COLOR_GREEN, new_string, ANSI_COLOR_RESET);
                    printf("\x1b[32m%s\x1b[0m\n", new_string);
                    no_file_match++;
                    strcpy(PATH_STORAGE, path);
                }
            }
            // if flagD is present only directories are printed
            if (FlagD == 1)
            {
                if (dirContent->d_type == 4)
                {
                    // sprintf(new_string, "%s%s%s", ANSI_COLOR_BLUE, new_string, ANSI_COLOR_RESET);
                    printf("\x1b[1;34m%s\x1b[0m\n", new_string);
                    no_dir_match++;
                    strcpy(PATH_STORAGE, path);
                }
            }
        }
        recurse_for_File(file, path, new_string, FlagF, FlagD);
    }
    closedir(dp);
}

// seek function
// method to implement seek command
int seek(char *args[])
{
    // initialize all the flags
    int arg_count = 0;
    int flagD = 0;
    int flagF = 0;
    int flagE = 0;
    int checkFlag = 0;
    int dirFlag = 0;
    int fileFlag = 0;
    int non_flag_count = 0;

    for (int i = 0; args[i] != NULL; i++)
    {
        arg_count++;
    }
    for (int i = 1; i < arg_count; i++)
    {
        if (!strcmp(args[i], "-d"))
        {
            flagD = 1;
        }
        else if (!strcmp(args[i], "-f"))
        {
            flagF = 1;
        }
        else if (!strcmp(args[i], "-e"))
        {
            flagE = 1;
        }
        else if (strcmp(args[i], "-d") && strcmp(args[i], "-f") && strcmp(args[i], "-e"))
        {
            non_flag_count++;
        }
    }

    // error managment (Invalid Flags!)
    if (flagD == 1 && flagF == 1)
    {
        printf("Invalid flags!\n");
        return -1;
    }
    if (flagD == 0 && flagF == 0)
    {
        flagD = 1;
        flagF = 1;
    }

    // // non_flag_count is the number of arguments
    // if(non_flag_count == 0)
    // {
    //     // pass current directory to recurse
    //     // file name also not provided
    //     char dir[PATH_MAX];
    //     strcpy(dir, ".");
    //     recursive_call = 0;
    //     recurse_dir(dir, flagF, flagD);
    // }
    // get current working directory
    char CWD[PATH_MAX];
    getcwd(CWD, PATH_MAX);

    if (non_flag_count == 1)
    {
        strcpy(KEYWORD, args[arg_count - 1]);
        strcpy(DIRECTORY, CWD);
        recursive_call = 0;
        no_file_match = 0;
        no_dir_match = 0;
        recurse_for_File(KEYWORD, DIRECTORY, ".", flagF, flagD);
        if ((no_file_match + no_dir_match) == 0)
        {
            printf("No match found!\n");
        }
        if ((no_file_match == 1) && (no_dir_match == 0))
        {
            if (flagE)
            {
                // read the contents of the file
                FILE *fp = fopen(PATH_STORAGE, "r");
                if (fp == NULL)
                {
                    printf("Missing permissions for task!");
                }
                else
                {
                    char c;
                    while ((c = fgetc(fp)) != EOF)
                    {
                        printf("%c", c);
                    }
                    fclose(fp);
                }
            }
        }
        if ((no_file_match == 0) && (no_dir_match == 1))
        {
            if (flagE)
            {
                // change working directory to the directory in PATH_STORAGE
                chdir(PATH_STORAGE);
            }
        }
    }
    else if (non_flag_count == 2)
    {

        strcpy(KEYWORD, args[arg_count - 2]);
        // handle the directory cases
        // handle directory flags
        if (!strcmp(args[arg_count - 1], "~"))
            strcpy(DIRECTORY, INVOC_LOC);
        else if (args[arg_count - 1][0] == '~')
        {
            // convert relative path to absolute path
            args[arg_count - 1] += 1;
            strcpy(DIRECTORY, INVOC_LOC);
            strcat(DIRECTORY, args[arg_count - 1]);
        }
        else if (!strcmp(args[arg_count - 1], "."))
        {
            // current working directory
            strcpy(DIRECTORY, CWD);
        }
        else if (!strcmp(args[arg_count - 1], "-"))
        {
            // previous location
            strcpy(DIRECTORY, PREV_LOC);
            printf("%s\n", PREV_LOC);
        }
        else if (!strcmp(args[arg_count - 1], ".."))
        {
            // parent directory
            char parent_path[PATH_MAX + 1];
            // get parent path
            const char *last_slash = strrchr(CWD, '/');
            // if last_slash is null, then we are in root directory
            if (last_slash == NULL)
            {
                strcpy(DIRECTORY, INVOC_LOC);
            }
            else
            {
                // copy the parent path into parent_path
                strncpy(parent_path, CWD, strlen(CWD) - strlen(last_slash));
                parent_path[strlen(CWD) - strlen(last_slash)] = '\0';
                strcpy(DIRECTORY, parent_path);
            }
        }
        else
        {
            // absolute path
            strcpy(DIRECTORY, args[arg_count - 1]);
        }

        // call recurse function
        recursive_call = 0;
        no_file_match = 0;
        no_dir_match = 0;
        recurse_for_File(KEYWORD, DIRECTORY, ".", flagF, flagD);
        if ((no_file_match + no_dir_match) == 0)
        {
            printf("No match found!\n");
        }
        if ((no_file_match == 1) && (no_dir_match == 0))
        {
            if (flagE)
            {
                // read the contents of the file
                FILE *fp = fopen(PATH_STORAGE, "r");
                if (fp == NULL)
                {
                    printf("Missing permissions for task!");
                }
                else
                {
                    char c;
                    while ((c = fgetc(fp)) != EOF)
                    {
                        printf("%c", c);
                    }
                    fclose(fp);
                }
            }
        }
        if ((no_file_match == 0) && (no_dir_match == 1))
        {
            if (flagE)
            {
                // change working directory to the directory in PATH_STORAGE
                chdir(PATH_STORAGE);
            }
        }
    }
    else
    {
        printf("seek: too many arguments\n");
    }
    return 0;
}