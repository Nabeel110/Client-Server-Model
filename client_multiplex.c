/*
 Copyright (c) 1986 Regents of the University of California.
 All rights reserved.  The Berkeley software License Agreement
 specifies the terms and conditions for redistribution.

    @(#)streamwrite.c    6.2 (Berkeley) 5/8/86
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
#include <sys/types.h>
#include <sys/time.h>

#define DATA "Half a league, half a league . . ."

/*
 * This program creates a socket and initiates a connection with the socket
 * given in the command line.  One message is sent over the connection and
 * then the socket is closed, ending the connection. The form of the command
 * line is streamwrite hostname portnumber 
 */

bool isConnected(int sock)
{
    int error = 0;
    socklen_t len = sizeof(error);
    int retval = getsockopt(sock, SOL_SOCKET, SO_ERROR, &error, &len);
    if (error != 0)
    {
        /* socket has a non zero error status */
        fprintf(stderr, "socket error: %s\n", strerror(error));
    }
    if (retval != 0)
    {
        /* there was a problem getting the error code */
        fprintf(stderr, "error getting socket error code: %s\n", strerror(retval));
        return false;
    }
    return true;
}
int main(int argc, char *argv[])
{
    int maxfdp;
    fd_set readfds;
    // fd_set writefds;
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
    server.sin_port = htons((atoi(argv[2])));

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("connecting stream socket");
        exit(1);
    }
    else
    {

        while (1)
        {

            bool isReading = true;
            // bzero(buf, sizeof(buf));
            // bzero(buf2, sizeof(buf));
            // write(STDOUT_FILENO, "Enter Command: ", strlen("Enter Command: "));
            int writeSucess = write(STDOUT_FILENO, "1.`add <list of numbers space separated>`\n2.`sub <list of numbers space separated>`\n3.`mult <list of numbers space separated>`\n4.`div <list of numbers space separated>`\n5.`run <program name>`\n6.`exit`\n7.`list` or `list all`\n8.`kill <name/pid>`\n",
                                    strlen("1.`add <list of numbers space separated>`\n2.`sub <list of numbers space separated>`\n3.`mult <list of numbers space separated>`\n4.`div <list of numbers space separated>`\n5.`run <program name>`\n6.`exit`\n7.`list` or `list all`\n8.`kill <name/pid>`\n"));

            if (writeSucess == -1)
            {
                perror("Error write");
            }

            char buff[1000] = {};
            while (isReading)
            {
                FD_ZERO(&readfds);
                FD_SET(STDIN_FILENO, &readfds);
                FD_SET(sock, &readfds);

                int max = -10;
                if (STDIN_FILENO > sock)
                {
                    max = STDIN_FILENO;
                }
                else
                {
                    max = sock;
                }
                maxfdp = max + 1;
                int ret = select(maxfdp, &readfds, NULL, NULL, NULL);
                if (ret == -1)
                {
                    perror("select");
                }

                if (FD_ISSET(STDIN_FILENO, &readfds))
                { //if input readable
                    int readSucesss = read(STDIN_FILENO, &buff, 1000);
                    if (readSucesss == -1)
                    {
                        perror("Error occured in line :44");
                    }
                    buff[readSucesss - 1] = '\0';
                    if (strcasecmp(buff, "connect") == 0)
                    {
                        if (isConnected(sock))
                        {
                            write(STDOUT_FILENO, "Already Connected\n", strlen("Already Connected\n"));
                        }
                        else
                        {
                            sock = 0;
                            if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
                            {
                                perror("connecting stream socket");
                                exit(1);
                            }
                        }
                    }
                    else if (strcasecmp(buff, "disconnect") == 0)
                    {
                        if (!isConnected(sock))
                        {
                            write(STDOUT_FILENO, "Already disconneted\n", strlen("Already disconneted\n"));
                        }
                        else
                        {
                            close(sock);
                        }
                    }
                    else
                    {
                        int ret = write(sock, buff, readSucesss);
                        if (ret == -1)
                        {
                            perror("Error while writing to pipe on line :50");
                        }
                    }
                }
                // write(STDOUT_FILENO, "Enter Command: ", strlen("Enter Command: "));

                if (FD_ISSET(sock, &readfds))
                {
                    char buff2[10000] = {};
                    int readResult = read(sock, &buff2, 10000);
                    if (readResult == -1)
                    {
                        if (errno == EINTR)
                        {
                            // isExited = true;
                            // close(fd);x
                            exit(0);
                        }
                        perror("Error while reading  result from socket on line");
                        exit(1);
                    }
                    else if (readResult == 0)
                    {
                        // isExited = true;
                        exit(0);
                    }
                    else if (readResult > 0)
                    {
                        if (strcmp(buff2, "exit") == 0)
                        {
                            isReading = false;
                            // isExited = true;
                            close(sock);
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
            // if (write(sock, DATA, sizeof(DATA)) < 0)
            //     perror("writing on stream socket");
            // getchar();
        }
    }

    close(sock);
}