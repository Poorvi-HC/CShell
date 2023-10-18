// this file has the implementation of peek
// peek is similar to "ls" command in Linux

// include custom libraries
#include "peek.h"

// variables declared elsewhere
extern char TEMP_BUFFER[];
extern char INVOC_LOC[];
extern char PREV_LOC[];
extern char SHELL_NAME[];
extern struct pData children[];
extern int childCount;
extern char foreProcess;
extern int exitCode;

// variables declared here
char DIR_PATH[PATH_MAX + 1 - MAX_FILENAME];
char FILE_PATH[PATH_MAX + 1];

// HEY BITCHHHH
// function to run peek functionality
void runPeek(char *args[])
{
    if (peek(args))
        handlePError(0);
}

// compare function for qsort (lexicographic order)
int compare(const void *a, const void *b)
{
    const char **ia = (const char **)a;
    const char **ib = (const char **)b;
    return strcmp(*ia, *ib);
}

int peek(char *args[])
{
    // obtain the current working directory
    char CWD[PATH_MAX + 1];
    getcwd(CWD, PATH_MAX);

    // count number of arguments in the command
    int argCount = 0;
    while (args[argCount] != NULL)
        argCount++;

    // check for the existance of aFlags and lFlags
    char aflag = 0;
    char lflag = 0;
    int no_flags = 0;
    for (int i = 0; i < argCount; i++)
    {
        if (!strcmp(args[i], "-a"))
        {
            aflag = 1;
            no_flags++;
        }
        if (!strcmp(args[i], "-l"))
        {
            lflag = 1;
            no_flags++;
        }
        if (!strcmp(args[i], "-la"))
        {
            lflag = 1;
            aflag = 1;
            no_flags++;
        }
        if (!strcmp(args[i], "-al"))
        {
            lflag = 1;
            aflag = 1;
            no_flags++;
        }
    }

    // Number of directories referenced in the command
    // more than one path can be referenced by the code
    int dirCount = argCount - no_flags - 1;
    // if no directories are referenced, use the current directory
    if (dirCount == 0)
        strcpy(args[0], ".");

    // run loop for every argument
    for (int i = 0; i < argCount; i++)
    {
        // if argument is neither "peek" nor beginning with "-", then it must be directory
        if (strcmp(args[i], "peek") && strcmp(args[i], "-l") && strcmp(args[i], "-a") && strcmp(args[i], "-al") && strcmp(args[i], "-la"))
        {
            // array to store maximum number of characters in each column of the -l implementation
            int columnWidth[7] = {0};

            // Open directory
            DIR *lsDir;

            // handle directory flags
            if (!strcmp(args[i], "~"))
                strcpy(DIR_PATH, INVOC_LOC);
            else if (args[i][0] == '~')
            {
                // convert relative path to absolute path
                args[i] += 1;
                strcpy(DIR_PATH, INVOC_LOC);
                strcat(DIR_PATH, args[i]);
            }
            else if (!strcmp(args[i], "."))
            {
                // current working directory
                strcpy(DIR_PATH, CWD);
            }
            else if (!strcmp(args[i], "-"))
            {
                // previous location
                strcpy(DIR_PATH, PREV_LOC);
                printf("%s\n", PREV_LOC);
            }
            else if (!strcmp(args[i], ".."))
            {
                // parent directory
                char parent_path[PATH_MAX + 1];
                // get parent path
                const char *last_slash = strrchr(CWD, '/');
                // if last_slash is null, then we are in root directory
                if (last_slash == NULL)
                {
                    strcpy(DIR_PATH, INVOC_LOC);
                }
                else
                {
                    // copy the parent path into parent_path
                    strncpy(parent_path, CWD, strlen(CWD) - strlen(last_slash));
                    parent_path[strlen(CWD) - strlen(last_slash)] = '\0';
                    strcpy(DIR_PATH, parent_path);
                }
            }
            else
            {
                // absolute path
                strcpy(DIR_PATH, args[i]);
            }

            lsDir = opendir(DIR_PATH);
            if (lsDir == NULL)
            {
                exitCode = -1;
                return -1;
            }
            // if more than one directory is present print the directory path before displaying the list
            if (dirCount > 1)
            {
                printf("%s:\n", DIR_PATH);
            }

            // Total size calculation logic in -l flags
            int total = 0;
            int index = 0;
            // outputs out the Directories and files under the current directory
            struct dirent *lsDirEntry = readdir(lsDir);
            // initialize an array of strings to store the list of directories and files under the given directory
            char **dirList = (char **)malloc(sizeof(char *) * MAX_DIRS);
            while (lsDirEntry != NULL)
            {
                // store lsDirEntry in dirList
                dirList[index] = (char *)malloc(sizeof(char) * MAX_FILENAME);
                strcpy(dirList[index], lsDirEntry->d_name);
                lsDirEntry = readdir(lsDir);
                index++;
            }

            // sort the dirList in lexicographic order
            qsort(dirList, index, sizeof(char *), compare);

            // logic for -l flag providing column widths for each entry
            for (int i = 0; i < index; i++)
            {
                if (lflag)
                {
                    sprintf(FILE_PATH, "%s/%s", DIR_PATH, dirList[i]);
                    struct stat fileStat;
                    if (lstat(FILE_PATH, &fileStat))
                    {
                        exitCode = -1;
                        return -1;
                    }
                    char hiddenFlag = dirList[i][0] == '.';
                    if ((aflag && hiddenFlag) || (!hiddenFlag))
                    {
                        columnWidth[0] = 10;
                        columnWidth[1] = max(columnWidth[1], digitCount((long long)fileStat.st_nlink));
                        columnWidth[2] = max(columnWidth[2], strlen(getpwuid(fileStat.st_uid)->pw_name));
                        columnWidth[3] = max(columnWidth[3], strlen(getgrgid(fileStat.st_gid)->gr_name));
                        columnWidth[4] = max(columnWidth[4], digitCount((long long)fileStat.st_size));
                        columnWidth[5] = -1;
                        columnWidth[6] = max(columnWidth[6], strlen(dirList[i]));
                        total += (fileStat.st_blocks * 512 + 1023) / 1024;
                    }
                }
            }

            // printing logic
            if (lflag)
            {
                printf("total %d\n", total);
            }
            for (int i = 0; i < index; i++)
            {
                // if -l flag is not given, print the list of directories and files
                if (!lflag)
                {
                    struct stat fileStat;
                    sprintf(FILE_PATH, "%s/%s", DIR_PATH, dirList[i]);

                    // if -a is given
                    char hiddenFlag = (dirList[i][0] == '.') ? 1 : 0;
                    // if there are no hidden files or -a flag is given
                    if ((hiddenFlag && aflag) || !hiddenFlag)
                    {
                        if (stat(FILE_PATH, &fileStat) == 0)
                        {
                            if (S_ISDIR(fileStat.st_mode))
                            {
                                // It's a directory, print it in blue
                                printf(ANSI_COLOR_BLUE "%s\n" ANSI_COLOR_RESET, dirList[i]);
                            }
                            else if (S_IXUSR & fileStat.st_mode || S_IXGRP & fileStat.st_mode || S_IXOTH & fileStat.st_mode)
                            {
                                // It's an executable, print it in red
                                printf(ANSI_COLOR_GREEN "%s\n" ANSI_COLOR_RESET, dirList[i]);
                            }
                            else
                            {
                                // It's a regular file, print it in white
                                printf(ANSI_COLOR_WHITE "%s\n" ANSI_COLOR_RESET, dirList[i]);
                            }
                        }
                        else
                        {
                            // There was an error accessing the file's information, print it in white as a fallback
                            printf(ANSI_COLOR_WHITE "%s\t" ANSI_COLOR_RESET, dirList[i]);
                        }
                    }
                }

                // if -l is given
                else
                {
                    // obtain path for each file
                    sprintf(FILE_PATH, "%s/%s", DIR_PATH, dirList[i]);
                    struct stat fileStat;
                    if (lstat(FILE_PATH, &fileStat) < 0)
                    {
                        exitCode = -1;
                        return -1;
                    }

                    // if along with -l we have -a flag
                    char hiddenFlag = (dirList[i][0] == '.');
                    if ((hiddenFlag && aflag) || (!hiddenFlag))
                    {
                        printFileDetails(dirList[i], fileStat, columnWidth);
                    }
                }

                // include a new line if multiple directories are provided as argument
                if (dirCount > 1)
                    printf("\n");
            }
            printf("\n");
            // Erase the extra new line that gets printed with -l
            if (lflag)
                printf("\x1B[A\b");
        }
    }
    // If multiple directories, erase the trailing newline
    if (dirCount > 1)
        printf("\x1B[A\b");
    return 0;
}

// method to print the file details when -l flag is provided
int printFileDetails(char *dir_name, struct stat fileStat, int columnWidth[])
{
    int currLen = 0;
    // permissions
    printFilePermissions(fileStat.st_mode);

    // number of links
    currLen = digitCount((long long)fileStat.st_nlink);
    for (int i = currLen; i < columnWidth[1]; i++)
        printf(" ");
    printf("%lu ", fileStat.st_nlink);

    // Owner left aligned
    printf("%s ", getpwuid(fileStat.st_uid)->pw_name);
    currLen = strlen(getpwuid(fileStat.st_uid)->pw_name);
    for (int i = currLen; i < columnWidth[2]; i++)
        printf(" ");

    // Group left aligned
    printf("%s ", getgrgid(fileStat.st_gid)->gr_name);
    currLen = strlen(getgrgid(fileStat.st_gid)->gr_name);
    for (int i = currLen; i < columnWidth[3]; i++)
        printf(" ");

    // File size right aligned
    currLen = digitCount((long long)fileStat.st_size);
    for (int i = currLen; i < columnWidth[4]; i++)
        printf(" ");
    printf("%lu ", fileStat.st_size);

    // Time, auto-formatted
    strcpy(TEMP_BUFFER, asctime(localtime(&(fileStat.st_mtime))));
    TEMP_BUFFER[strlen(TEMP_BUFFER) - 1] = '\0';
    printf("%s ", TEMP_BUFFER);

    // File name
    // printf("%s\n", dir_name);

    if (stat(FILE_PATH, &fileStat) == 0)
    {
        if (S_ISDIR(fileStat.st_mode))
        {
            // It's a directory, print it in blue
            printf(ANSI_COLOR_BLUE "%s\n" ANSI_COLOR_RESET, dir_name);
        }
        else if (S_IXUSR & fileStat.st_mode || S_IXGRP & fileStat.st_mode || S_IXOTH & fileStat.st_mode)
        {
            // It's an executable, print it in red
            printf(ANSI_COLOR_GREEN "%s\n" ANSI_COLOR_RESET, dir_name);
        }
        else
        {
            // It's a regular file, print it in white
            printf(ANSI_COLOR_WHITE "%s\n" ANSI_COLOR_RESET, dir_name);
        }
    }
    else
    {
        // There was an error accessing the file's information, print it in white as a fallback
        printf(ANSI_COLOR_WHITE "%s\n" ANSI_COLOR_RESET, dir_name);
    }

    return 0;
}

// this method prints the file permissions
void printFilePermissions(mode_t bits)
{
    char permString[] = " ---------";
    if (S_ISDIR(bits))
        permString[0] = 'd';
    else if (S_ISLNK(bits))
        permString[0] = 'l';
    else if (S_ISBLK(bits))
        permString[0] = 'b';
    else if (S_ISCHR(bits))
        permString[0] = 'c';
    else if (S_ISSOCK(bits))
        permString[0] = 's';
    else if (S_ISFIFO(bits))
        permString[0] = 'f';
    else if (S_ISREG(bits))
        permString[0] = '-';
    else
        permString[0] = '?';

    if (bits & S_IRUSR)
        permString[1] = 'r';
    if (bits & S_IWUSR)
        permString[2] = 'w';
    if (bits & S_IXUSR)
        permString[3] = 'x';
    if (bits & S_IRGRP)
        permString[4] = 'r';
    if (bits & S_IWGRP)
        permString[5] = 'w';
    if (bits & S_IXGRP)
        permString[6] = 'x';
    if (bits & S_IROTH)
        permString[7] = 'r';
    if (bits & S_IWOTH)
        permString[8] = 'w';
    if (bits & S_IXOTH)
        permString[9] = 'x';

    printf("%s ", permString);
}
