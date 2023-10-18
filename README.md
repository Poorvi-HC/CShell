# Description
## Building our own shell using C

## Modular code:
### File structure:
    - src
        - definitions.h
        - cshell.c (main.c file)
        - utilities.h
        - utilities.c
        - commands
            - *.c
            - *.h
    - MakeFile
    - README.md


## How to run the shell:

1. clone the repository
1. enter the CSHELL folder
1. Run the below commands

```
    $ make all
    $ ./cshell
```


# Assumptions

1. absolute path - with respect to the local system file system
1. relative path - can handle ./, ../, and ~/ alone
1. Command specific
    - For warp
        - absolute paths are wrt local system
        - .. from invocation direcotry moves it to the parent directory of the invocation loc
    - For peek
        - displayed as files and folders in new line
        - color scheme: green for executables, white for files and blue for directories
    - For seek:
        - not lexicographically ordered 
        - color scheme: files in green and directories in blue
        - all error flags needed included
        - shell crashes when searching for some file in the directory
    - For system calls:
        - pid sometimes prints after generation of the Prompt string
        - A new prompt string is generated once the child process running in the background exits/ is killed.
    - For sed:
        - sed doesn't take quotational arguments
    - proclore:
        - no assumptions made
    - pastevents:
        - no assumptions made
    - fg & bg:
        - only stopped processes can resume in bg
        - have handled float and negative numbers alone
    - piping:
        - haven't handled the case where piped commands are placed in the background.
    - neonate:
        - 'x' key when pressed, might have some delay for exiting the command
    - iman:
        - am printing all the manual contents in the shell 
        - some additional html tags aren't handled 
    - activities:
        - prints in lexicographical order of job number
    - ping:
        - no assumptions made

1. Exit shell using "exit" or "quit" command
1. signals - 
    Ctrl-Z -> moves into background
    Ctrl-C -> exits the running foreground process
    Ctrl-D -> exist shell