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
#include <poll.h>

/*
 * This program creates a socket and initiates a connection with the socket
 * given in the command line.  One message is sent over the connection and
 * then the socket is closed, ending the connection. The form of the command
 * line is streamwrite hostname portnumber 
 */

int main(int argc, char *argv[])
{
    struct pollfd fds[2];
    int ret;

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
        /* watch stdin for input */
        fds[0].fd = STDIN_FILENO;
        fds[0].events = POLLIN;
        fds[1].fd = sock;
        fds[1].events = POLLIN;
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

                ret = poll(fds, 2, -1);
                if (ret == -1)
                {
                    perror("poll");
                }

                if (fds[0].revents & POLLIN)
                { //if input readable
                    int readSucesss = read(STDIN_FILENO, &buff, 1000);
                    if (readSucesss == -1)
                    {
                        perror("Error occured in line :44");
                    }
                    else if (readSucesss == 0)
                    {
                        exit(0);
                    }
                    buff[readSucesss - 1] = '\0';
                    int ret = write(sock, buff, readSucesss);
                    if (ret == -1)
                    {
                        perror("Error while writing to pipe on line :50");
                    }
                }
                // write(STDOUT_FILENO, "Enter Command: ", strlen("Enter Command: "));

                if (fds[1].revents & POLLIN)
                {
                    char buff2[10000] = {};
                    int readResult = read(sock, &buff2, 10000);
                    if (readResult == -1)
                    {
                        if (errno == EINTR)
                        {
                            // isExited = true;
                            // close(fd);
                            exit(0);
                        }
                        perror("Error while reading  result from socket on line");
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