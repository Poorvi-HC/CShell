// contains implementation of iman functionality
// iman - opens the man page of the command provided by the user
// command structure:
//     iman <command>

// include custom header files
#include "iman.h"

// variables defined elsewhere
extern char exitCode;

// method to run the iman command
int run_iMan(char* args[])
{
    if (iMan(args))
        handlePError(0);
    return 0;
}

// Method to print the parsed content of the man page
void printManPageContent(const char* content) {
    // Implement your HTML parsing logic here to extract relevant information.
    // You can use libraries like libxml2 or write custom parsing code.
    // For simplicity, let's assume you've extracted the relevant content
    // and stored it in the 'content' variable.
    
    if (strlen(content) > 0) {
        printf("%s", content);
        printf("\n");
    } else {
        printf("ERROR\n\tNo man page exists for the command\n");
    }
}


// method to implement iman
int iMan(char* args[])
{
    // check for number of arguments
    int count_args = 0;
    while(args[count_args] != NULL)
        count_args++;
    
    if (count_args != 2)
    {
        fprintf(stderr, "iMan: Invalid number of arguments. SYNTAX: iMan <command>\n");
        handlePError(0);
        exitCode = -1;
        return 1;
    }

    // get the command
    char command[100];
    strcpy(command, args[1]);

    // Resolve DNS for "man.he.net"
    struct hostent* host_info = gethostbyname("man.he.net");
    if (host_info == NULL) {
        fprintf(stderr, "Error: Cannot resolve DNS\n");
        handlePError(0);
        exitCode = -1;
        return 1;
    }

    // Create a socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("Error creating socket");
        return 1;
    }

    // Prepare server address
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(80);
    server_addr.sin_addr = *((struct in_addr*)host_info->h_addr);

    // Connect to the server
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error connecting to server");
        close(sockfd);
        return 1;
    }

     // Construct the HTTP GET request
    char request[256];
    snprintf(request, sizeof(request), "GET /?topic=%s&section=all HTTP/1.1\r\nHost: man.he.net\r\n\r\n", command);

     // Send the request
    if (send(sockfd, request, strlen(request), 0) == -1) {
        perror("Error sending request");
        close(sockfd);
        return 1;
    }

    // Read and print the response
    char response[MAX_RESPONSE_SIZE];
    char parsedContent[MAX_RESPONSE_SIZE];
    parsedContent[0] = '\0';

    ssize_t bytes_received;
    int in_body = 0; // flag to track if we are inside body tag
    int no_matches = 0; // flag to check for matches

    while ((bytes_received = recv(sockfd, response, sizeof(response) - 1, 0)) > 0) {
        response[bytes_received] = '\0';

        // find the start of the <body> tag
        if(!in_body)
        {
            char* body_start = strstr(response, "<STRONG");
            char* body_end = NULL;
            if(body_start != NULL)
            {
                in_body = 1;
                char* body_content = strstr(body_start, ">");
                if (body_content != NULL)
                {
                    body_content++;
                    strcat(parsedContent, body_content);
                    body_end = strstr(body_content, "<STRONG");
                    if(body_end != NULL)
                    {
                        body_end = strstr(body_end, ">");
                        strcat(parsedContent,"\0");
                        break;
                    }
                }     
            }
        }
        else{
            if(strstr(response, "No matches") != NULL){
                no_matches = 1;
                break;
            }
            // strcat(parsedContent, "No man page exists for the command\n");
        }
    }

    // Close the socket
    close(sockfd);

    // Print the parsed content
    printManPageContent(parsedContent);

    return 0;
}