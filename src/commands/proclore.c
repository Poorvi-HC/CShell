// this is used to implement proclore
// this function is identical to the "pinfo" function in Linux

// custom header files
#include "proclore.h"

// variables defined elsewhere
extern char exitCode;
extern char INVOC_LOC[];

// method to run the proclore command
int runProclore(char* args[])
{
    if (proclore(args))
        handlePError(0);
    return 0;
}

int isForeground(pid_t pid){
    pid_t pgid = getpgid(pid);
    if(getpgid(pid) == -1){
        return -1;
    }
    return pgid == pid;
}

// method used to implement proclore
int proclore(char* args[])
{
    // Get required process id
    pid_t pid = 0;
    if (args[1] != NULL)
        pid = (pid_t)atoi(args[1]);
    if (pid == 0)
        pid = getpid();
    
    // Open process file and read contents
    char procFile[20];
    // place the path of proc file in proc File
    // place the contents of proc file in procFileStat buffer
    sprintf(procFile, "/proc/%d/stat", (int)pid);
    char procFileStat[MAX_STAT_LEN + 1] = {'\0'};

    // check if Plus is to be added or not
    const char *isPlus = isForeground(pid) ? "+" : "";
    // read procStat file and handle error
    FILE* procStat;
    procStat = fopen(procFile, "r");
    if (procStat == NULL)
    {
        errno = ESRCH;
        exitCode = -1;
        return -1;
    }

    // read procStat file (if there exists some stats)
    fread(procFileStat, MAX_STAT_LEN, 1, procStat);
    fclose(procStat);

    // place a pointer for every space seperated stat in the file
    char* attributes[STAT_COUNT] = {NULL};
    attributes[0] = strtok(procFileStat, " ");
    // attrs[i-1] content is  present then only the next stat can be taken in
    for(int i = 1;attributes[i - 1] != NULL;i++)
        attributes[i] = strtok(NULL, " ");
    
    // Read procExec file and handle error
    sprintf(procFile, "/proc/%d/exe", (int)pid);
    char execPath[PATH_MAX + 1];
    memset(execPath, 0, PATH_MAX);
    int readExec = readlink(procFile, execPath, PATH_MAX);

    // if Zombie process, exe doesn't exist, prompt
    if (readExec != -1)
        shortenPathSubDir(INVOC_LOC, execPath);
    
    // print pid , Process Status, Memory and Exec_path
    if (readExec == -1)
        strcpy(attributes[2],"Z");
        
    printf("pid : %d\n", pid);
    printf("process Status : %s%s\n", attributes[2], isPlus);
    printf("Process Group : %s\n", attributes[4]);
    printf("Virtual memory : %s\n", attributes[22]);
    printf("Executable Path : %s\n", readExec == -1 ? "Doesn't exist" : execPath);
    
    return 0;
}