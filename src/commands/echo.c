// implements the echo command

// custom header files
#include "echo.h"

// method to implement echo command
void echo(char* args[])
{
    for (int i = 1; args[i] != NULL;i++)
    {
        printf("%s ", args[i]);
    }
    printf("\n");
}
