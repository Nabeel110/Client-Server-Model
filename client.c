/*
 Copyright (c) 1986 Regents of the University of California.
 All rights reserved.  The Berkeley software License Agreement
 specifies the terms and conditions for redistribution.

	@(#)streamwrite.c	6.2 (Berkeley) 5/8/86
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>

#include <sys/fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <pthread.h>
// #include <"unp.h">
#define SERVER_PORT 37305

/*
 * This program creates a socket and initiates a connection with the socket
 * given in the command line.  One message is sent over the connection and
 * then the socket is closed, ending the connection. The form of the command
 * line is streamwrite hostname portnumber 
 */

bool isReading = true;

bool validInput(char *token)
{
    bool valid = true;

    int length = strlen(token);
    for (int i = 0; i < length; i++)
    {
        if (!isdigit(*token))
        {
            valid = false;
        }
        token = token + 1;
    }

    return valid;
}

void *displayOutput(void *input)
{
    bool isExited = false;
    int fd = *((int *)input);

    // write(STDOUT_FILENO,)
    while (!isExited)
    {
        char buff2[10000] = {};
        int readResult = read(fd, &buff2, 10000);
        if (readResult == -1)
        {
            if (errno == EINTR)
            {
                isExited = true;
                // close(fd);
                exit(0);
            }
            perror("Error while reading  result from socket on line");
        }
        else if (readResult == 0)
        {
            isExited = true;
            exit(0);
        }
        else if (readResult > 0)
        {
            if (strcmp(buff2, "exit") == 0)
            {
                isReading = false;
                isExited = true;
                close(fd);
                exit(EXIT_SUCCESS);
            }
            write(STDOUT_FILENO, "Recieved from server: ", strlen("Recieved from server: "));
            int ret = write(STDOUT_FILENO, buff2, readResult);
            if (ret == -1)
            {
                perror("error while writing result: ");
            }
        }
    }
}
int main(int argc, char *argv[])
{
    pthread_t thread1;
    int sock;
    struct sockaddr_in server;
    struct hostent *hp;
    char buf[1024];
    char buf2[1024];

    /* Create socket */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("opening stream socket");
        exit(1);
    }
    /* Connect socket using name specified by command line. */
    server.sin_family = AF_INET;
    hp = gethostbyname(argv[1]);
    if (hp == 0)
    {
        fprintf(stderr, "%s: unknown host\n", argv[1]);
        exit(2);
    }
    bcopy(hp->h_addr, &server.sin_addr, hp->h_length);
    server.sin_port = htons(atoi(argv[2]));

    // htons(atoi(argv[2]))
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("connecting stream socket");
        exit(1);
    }
    else
    {

        while (1)
        {

            //variable to store thread attribute
            pthread_attr_t attr;
            pthread_attr_init(&attr); // initializing pthread with default attributes

            int retDetached = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
            if (retDetached > 0)
            {
                perror("Error while creating Thread");
            }
            else if (retDetached == 0)
            {
                // write(STDOUT_FILENO, "Success", strlen("Success"));
            }
            //Creating thread in detachable form
            pthread_create(&thread1, &attr, &displayOutput, (void *)&sock);

            int writeSucess = write(STDOUT_FILENO, "\t\t\t***** Client-Side Commands *****\n\t\t\t\t By Nabeel Ahmed\n\n1.`add <list of numbers space separated>`\n2.`sub <list of numbers space separated>`\n3.`mult <list of numbers space separated>`\n4.`div <list of numbers space separated>`\n5.`run <program name>`\n6.`exit`\n7.`list` or `list all`\n8.`kill <name/pid>`\n",
                                    strlen("\t\t\t***** Client-Side Commands *****\n\t\t\t\t By Nabeel Ahmed\n\n1.`add <list of numbers space separated>`\n2.`sub <list of numbers space separated>`\n3.`mult <list of numbers space separated>`\n4.`div <list of numbers space separated>`\n5.`run <program name>`\n6.`exit`\n7.`list` or `list all`\n8.`kill <name/pid>`\n"));

            if (writeSucess == -1)
            {
                perror("Error write");
            }

            char buff[1000] = {};
            while (isReading)
            {
                // write(STDOUT_FILENO, "Enter Command: ", strlen("Enter Command: "));
                int readSucesss = read(STDIN_FILENO, &buff, 1000);
                if (readSucesss == -1)
                {
                    perror("Error occured in line :44");
                }
                if (readSucesss == 0)
                {
                    exit(0);
                }
                buff[readSucesss - 1] = '\0';
                int ret = write(sock, buff, readSucesss);
                // perror(ret);
                if (ret == -1)
                {

                    perror("Error while writing to pipe on line :50");
                    exit(0);
                }
            }
        }
    }
    // close(sock);
}
