

/*
 Copyright (c) 1986 Regents of the University of California.
 All rights reserved.  The Berkeley software License Agreement
 specifies the terms and conditions for redistribution.

	@(#)streamread.c	6.2 (Berkeley) 5/8/86
*/
#define _GNU_SOURCE
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
#include <arpa/inet.h>
#define SERVER_PORT 37305

/*
 * This program creates a socket and then begins an infinite loop. Each time
 * through the loop it accepts a connection and prints out messages from it. 
 * When the connection breaks, or a termination message comes through, the
 * program accepts a new connection. 
 */

struct Process
{
    int num;
    int pid;
    char procName[100];
    bool isActive;
    time_t start;
    time_t endt;
};
//co . c1
struct ClientInformation
{
    char client_id[20];
    int pid;
    int sock;
    int addr_len;
    char ipAddress[200];
    int client_port;
    int pipefd_p[2];
    int pipefd_c[2];
    bool isActive;
};
// Global Variables
int sno = 0;
int connectionCount = 0;
struct Process proclist[2000];
struct ClientInformation clientList[1000] = {0};

//================================= Signal Handler =======================================//
void signal_handler(int signo)
{
    int status;

    if (signo == SIGCHLD)
    {
        int pid = waitpid(0, &status, WNOHANG);
        while (pid != 0) //-1 when no child  //0 when child present but not terminated(not any child termicated)
        {
            for (int i = 0; i < sno; i++)
            {
                if (proclist[i].pid == pid)
                {
                    proclist[i].isActive = false;
                    proclist[i].endt = time(NULL);
                }
            }
            for (int i = 0; i < connectionCount; i++)
            {
                // int msg[100]={};
                // int ret = sprintf(msg,"Shutting child Process: %s",cl)
                if (clientList[i].pid == pid)
                {
                    clientList[i].isActive = false;
                }
            }
            pid = waitpid(0, &status, WNOHANG); // 0 means child is present but not exited
            if (pid == -1)
            {
                if (errno != ECHILD)
                {
                    perror("Wait-PID Error");
                }
                break;
            }
        }
    }
    else if (signo == SIGTERM || signo == SIGINT)
    {
        for (int i = 0; i < sno; i++)
        {
            if (proclist[i].isActive)
            {
                int status;
                int ret = kill(proclist[i].pid, SIGTERM);
                if (ret == -1)
                {
                    perror("Error while Killing");
                }
            }
        }
        exit(0);
    }
}

//====================================  ********************** ==============================

//==================================== Validate Input ======================================
/*
    Function: To validate Input givrn from client
    Input: token
    Output: boolean: valid ? true: false
*/

// * /
bool validInput(char *token)
{
    int i = 0;
    int length = strlen(token);
    while (length != 0)
    {
        if (!isdigit(token[i]))
        {
            if (token[i] != '-')
            {
                return false;
            }
            else
            {
                if (i == length - 1)
                    return false;
            }
            if (token[i] == '-' && i == length)
                return false;
        }
        i++;
        length--;
    }
    return true;
}

//====================================  ********************** ==============================

//====================================  Read & Write Errro-Checks ===========================

/*
    Function: Read & Write Errro-Checks
    Input:    ret value
    Output:   Displays perror if error == -1
*/
void checkRead(int ret)
{
    if (ret == -1)
    {
        perror("Error in read");
    }
}

void checkWrite(int ret)
{
    if (ret == -1)
    {
        perror("Error in write");
    }
}

void checkSprintf(int ret)
{
    if (ret == -1)
    {
        perror("Error in sprintf");
    }
}

//====================================  ********************** ==============================

//====================================  Validate ClientHandler ID ===========================

/*
    Function: idValidClientID
    Input: Client ID
    Output: validId ? true  :false
*/
bool isValidClientId(char *token)
{
    bool valid = false;
    int len = strlen(token);

    for (int i = 0; i < connectionCount; i++)
    {
        if (strcasecmp(clientList[i].client_id, token) == 0)
        {
            valid = true;
            break;
        }
    }
    return valid;
}

//====================================  ********************** ==============================

//====================================  checkIsAllClientInactive ===========================

/*
    Function: checkIsAllClientInactive
    Input:  - -
    Output: isInActive ? true  :false
    Desc  : It iterates over all the list to see if all connections are closed or not.
*/
bool checkIsAllClientInactive()
{
    bool isInactive = true;

    for (int i = 0; i < connectionCount; i++)
    {
        if (clientList[i].isActive)
        {
            isInactive = false;
            break;
        }
    }
    return isInactive;
}

//====================================  ********************** ==============================

//====================================  Print All processes List =============================
/*
    Function: printlist
    Input: processList, fd
    Output: prints all processList active and inactive both
*/
void printlist(struct Process process[], int fd)
{
    // char concatBuff[10000] = {};
    char finalConcatBuff[10000] = {};
    char finalResult[10000] = {};

    int ret, ret1;
    char st_time[200] = {};
    char end_time[200] = {};
    ret = sprintf(finalConcatBuff, "\nID\tPID\tPName\tActive\tStart-Time\tEnd-time\tElapsed-time\n");
    checkSprintf(ret);
    for (int i = 0; i < sno; i++)
    {
        char concatBuff[10000] = {};
        if (process[i].start != 0)
        {
            struct tm TM = *localtime(&process[i].start);

            // //Must add 1 to month and 1900 to the year
            int hours = TM.tm_hour;
            int min = TM.tm_min;
            int sec = TM.tm_sec;
            int ret3 = sprintf(st_time, "%d:%d:%d", hours, min, sec);
            checkSprintf(ret3);
        }
        if (!process[i].isActive)
        {
            struct tm TM = *localtime(&process[i].endt);

            // //Must add 1 to month and 1900 to the year
            int hours = TM.tm_hour;
            int min = TM.tm_min;
            int sec = TM.tm_sec;
            ret = sprintf(end_time, "%d:%d:%d", hours, min, sec);
            checkSprintf(ret);
            // ret = sprintf(concatBuff, "Pno: %d Pid: %d Pname: %s isActive: %d Start-Time: %s End-Time: %s Elapsed-Time: %.2f\n", process[i].num, process[i].pid, process[i].procName, process[i].isActive, st_time, end_time, difftime(process[i].endt, process[i].start));
            ret = sprintf(concatBuff, "%d\t%d\t%s\t %d\t  %s\t%s\t\t%.2fs\n", process[i].num, process[i].pid, process[i].procName, process[i].isActive, st_time, end_time, difftime(process[i].endt, process[i].start));
            checkSprintf(ret);
        }

        else
        {
            ret = sprintf(concatBuff, "%d\t%d\t%s\t %d\t  %s\t%s\t\t%s\n", process[i].num, process[i].pid, process[i].procName, process[i].isActive, st_time, "-", "-");
            checkSprintf(ret);
        }

        strcat(finalResult, concatBuff);
        // strcat(finalConcatBuff, finalResult);
        // perror(resultBuff);
    }
    strcat(finalConcatBuff, finalResult);
    if (strlen(finalResult) == 0)
    {
        ret = write(fd, "No Proccess Created\n", strlen("No Proccess Created\n"));
        checkWrite(ret);
    }
    else
    {
        ret = write(fd, finalConcatBuff, strlen(finalConcatBuff));
        checkWrite(ret);
    }
}

//====================================  ********************** ==============================

//====================================  Print Active processes List ==========================
/*
    Function: printlist
    Input: processList, fd
    Output: prints active processList active and inactive both
*/
void printActivelist(struct Process process[], int msgsock)
{
    char concatBuff[10000] = {};
    char finalResult[10000] = {};
    char finalConcatBuff[10000] = {};
    char st_time[200] = {};
    int ret;

    ret = sprintf(finalConcatBuff, "\nID\tPID\tPName\tActive\tStart-Time\tEnd-time\tElapsed-time\n");
    checkSprintf(ret);
    // write(STDOUT_FILENO, "ID\tPID\tPName\t\t\tActive\tStart-Time\tEnd-time\tElapsed-time\n", strlen("ID\tPID\tPName\t\t\tActive\tStart-Time\tEnd-time\tElapsed-time\n"));
    for (int i = 0; i < sno; i++)
    {
        if (process[i].start != 0)
        {
            struct tm TM = *localtime(&process[i].start);

            // //Must add 1 to month and 1900 to the year
            int hours = TM.tm_hour;
            int min = TM.tm_min;
            int sec = TM.tm_sec;
            ret = sprintf(st_time, "%d:%d:%d", hours, min, sec);
            checkSprintf(ret);
        }
        if (process[i].isActive)
        {
            ret = sprintf(concatBuff, "%d\t%d\t%s\t %d\t  %s\t%s\t\t%s\n", process[i].num, process[i].pid, process[i].procName, process[i].isActive, st_time, "-", "-");
            checkSprintf(ret);
            // ret = sprintf(concatBuff, "Pno: %d Pid: %d Pname: %s isActive: %d Start-Time: %s End-Time: %s Elapsed-Time: %s\n", process[i].num, process[i].pid, process[i].procName, process[i].isActive, st_time, "-", "-");
            // checkSprintf(ret);
            strcat(finalResult, concatBuff);
        }
    }
    strcat(finalConcatBuff, finalResult);
    if (strlen(finalResult) == 0)
    {
        ret = write(msgsock, "No Active Processes\n", strlen("No Active Processes\n"));
        checkWrite(ret);
    }
    else
    {
        ret = write(msgsock, finalConcatBuff, strlen(finalConcatBuff));
        checkWrite(ret);
    }
}
//====================================  ********************** ==============================

//====================================  Display Client List ==================================
/*
    Function: displayClientList
    Input:           -
    Output:   prints all connecctions established with their properties which is stored after accept.
*/
void displayClientList()
{
    int size = sizeof clientList / sizeof clientList[0];
    if (connectionCount > 0)
    {
        int ret = write(STDOUT_FILENO, "CID\t PID\t ClientIP\t ClientPort\t isActive\n", strlen("CID\t PID\t ClientIP\t ClientPort\t isActive\n"));
        checkWrite(ret);
        for (int i = 0; i < connectionCount; i++)
        {
            char clientInfo[500] = {};
            int ret = sprintf(clientInfo, "%s\t %d\t %s\t %d\t\t  %d\n", clientList[i].client_id, clientList[i].pid, clientList[i].ipAddress, clientList[i].client_port, clientList[i].isActive);
            checkSprintf(ret);
            ret = write(STDOUT_FILENO, clientInfo, strlen(clientInfo));
            checkWrite(ret);
        }
    }
    else
    {
        int ret = write(STDOUT_FILENO, "No Connections have been established yet.\n", strlen("No Connections have been established yet.\n"));
        checkWrite(ret);
    }
}
//====================================  ********************** ==============================

//================================  Thread:1 takeuserInput(From connection) ================
/*
    Function: takeuserInput
    Input:    void *param ->
    Output:    -
    Desc:     This Thread is created in Connection to continuously take input from user.
*/
void *takeuserInput(void *param)
{
    char *context = NULL;
    bool stop = false;

    int ret = write(STDOUT_FILENO, "\n\t\t\t  ***Server-Side Commands***\n\t\t\t\tBy Nabeel Ahmed\n\n1.connlist\n2.list\n3.list <clientId>\n4.listall\n5.listall <clientId>\n6.print <message>\n7.print <clientid> message\n", strlen("\n\t\t\t  ***Server-Side Commands***\n\t\t\t\tBy Nabeel Ahmed\n\n1.connlist\n2.list\n3.list <clientId>\n4.listall\n5.listall <clientId>\n6.print <message>\n7.print <clientid> message\n"));
    checkWrite(ret);
    while (!stop)
    {
        char buff[100] = {};
        char token1[100] = {};
        char token2[100] = {};
        char token3[1000] = {};
        char inputCommand[100] = {};
        char finalMessage[1000] = {};
        int readRet = read(STDIN_FILENO, buff, sizeof(buff));
        // buff[readRet - 1] = '\0';
        strcpy(inputCommand, buff);
        if (readRet == -1)
        {
            perror("Error");
        }
        char *token = strtok_r(buff, " \n", &context);
        if (token == NULL)
        {
            int ret = write(STDOUT_FILENO, "No Command passed\n", strlen("No Command passed\n"));
            checkWrite(ret);
            continue;
        }
        else if (strcasecmp(token, "print") == 0)
        {

            strcpy(token1, token);
            token = strtok_r(NULL, " \n", &context);
            if (token == NULL)
            {
                int ret = write(STDOUT_FILENO, "No message given after print.\n", strlen("No message given after print.\n"));
                checkWrite(ret);
            }
            else
            {
                strcpy(token2, token);
                if (isValidClientId(token2))
                {
                    token = strtok_r(NULL, "\n", &context);
                    if (token == NULL)
                    {
                        int ret = write(STDOUT_FILENO, "No message given to sent to client.\n", strlen("No message given to sent to client.\n"));
                        checkWrite(ret);
                    }
                    else
                    {
                        strcpy(token3, token);
                        char finalMessage[1000];
                        int ret = sprintf(finalMessage, "%s %s", token1, token3);
                        for (int i = 0; i < connectionCount; i++)
                        {
                            if (strcasecmp(clientList[i].client_id, token2) == 0)
                            {
                                if (clientList[i].isActive)
                                {
                                    int ret = write(clientList[i].pipefd_p[1], finalMessage, strlen(finalMessage));
                                    checkWrite(ret);
                                    break;
                                }
                                else
                                {
                                    int ret = write(STDOUT_FILENO, "Client is not active\n", strlen("Client is not active\n"));
                                    checkWrite(ret);
                                }
                            }
                        }
                    }
                }
                else
                {
                    token = strtok_r(NULL, "\n", &context);
                    if (token == NULL)
                    {
                        int ret = sprintf(finalMessage, "%s %s", token1, token2);
                        for (int i = 0; i < connectionCount; i++)
                        {
                            if (clientList[i].isActive)
                            {
                                int ret = write(clientList[i].pipefd_p[1], finalMessage, strlen(finalMessage));
                                checkWrite(ret);
                            }
                        }
                    }
                    else
                    {
                        strcpy(token3, token);
                        int ret = sprintf(finalMessage, "%s %s %s", token1, token2, token3);
                        for (int i = 0; i < connectionCount; i++)
                        {
                            if (clientList[i].isActive)
                            {
                                int ret = write(clientList[i].pipefd_p[1], finalMessage, strlen(finalMessage));
                                checkWrite(ret);
                            }
                        }
                    }
                }
            }
        }
        else if (strcasecmp(token, "list") == 0)
        {
            /*
                1. Check if any client ID Passed.
                2. if no clcient ID passed sent list to all clientHandlers
                3. Otherwise send command to that particlcuar ClinetHandler
            */
            strcpy(token1, token);
            token = strtok_r(NULL, " \n", &context);

            if (token == NULL)
            {
                char accumulatedList[10000] = {};
                bool hasActiveClients = false;
                for (int i = 0; i < connectionCount; i++)
                {
                    char locaList[1000] = {};
                    char localConcatList[1000] = {};
                    if (clientList[i].isActive)
                    {
                        hasActiveClients = true;
                        int ret = write(clientList[i].pipefd_p[1], token1, strlen(token1));
                        checkWrite(ret);
                        int readRet = read(clientList[i].pipefd_c[0], &locaList, 10000);
                        checkRead(ret);
                        if (readRet > 0)
                        {
                            int ret = sprintf(localConcatList, "\t========== Client-ID: %s AND ClientIP: %s ==========\n%s\n \t\t\t===========================\n", clientList[i].client_id, clientList[i].ipAddress, locaList);
                            checkSprintf(ret);
                            strcat(accumulatedList, localConcatList);
                        }
                    }
                }
                if (hasActiveClients)
                {
                    int ret = write(STDOUT_FILENO, accumulatedList, strlen(accumulatedList));
                    checkWrite(ret);
                }
                else
                {
                    int ret = write(STDOUT_FILENO, "No Active Connections\n", strlen("No Active Connections\n"));
                    checkWrite(ret);
                }
            }
            else
            {

                strcpy(token2, token);
                token = strtok_r(NULL, " ", &context);
                if (token == NULL)
                {
                    char returnList[10000] = {};
                    if (isValidClientId(token2))
                    {
                        bool isClientActive = false;
                        char cid[20];
                        strcpy(cid, token2);
                        for (int i = 0; i < connectionCount; i++)
                        {
                            if ((strcasecmp(clientList[i].client_id, cid) == 0) && clientList[i].isActive)
                            {
                                isClientActive = true;
                                int ret = write(clientList[i].pipefd_p[1], token1, strlen(token1));
                                checkWrite(ret);
                                int readRet = read(clientList[i].pipefd_c[0], &returnList, 10000);
                                checkRead(ret);

                                if (readRet > 0)
                                {
                                    char printResultList[10000];
                                    int ret = sprintf(printResultList, "\t========== Client-ID: %s AND ClientIP: %s ==========\n%s\n \t\t\t===========================\n", clientList[i].client_id, clientList[i].ipAddress, returnList);
                                    int ret1 = write(STDOUT_FILENO, printResultList, ret);
                                    checkWrite(ret);
                                    break;
                                }
                                break;
                            }
                        }
                        if (!isClientActive)
                        {
                            int ret = write(STDOUT_FILENO, "Client is not active\n", strlen("Client is not active\n"));
                            checkWrite(ret);
                        }
                    }
                    else
                    {
                        int ret = write(STDOUT_FILENO, "Invalid Client ID\n", strlen("Invalid Client ID\n"));
                        checkWrite(ret);
                        continue;
                    }
                }
                else
                {
                    int ret = write(STDOUT_FILENO, "Invalid input passed after client ID\n", strlen("Invalid input passed after client ID\n"));
                    checkWrite(ret);
                }
            }
        }
        else if (strcasecmp(token, "listall") == 0)
        {
            strcpy(token1, token);
            token = strtok_r(NULL, " ", &context);
            char tempBuff[100] = {};
            int ret = sprintf(tempBuff, "%s", token);
            tempBuff[ret - 2] = '\0';
            tempBuff[ret - 1] = '\0';
            if (tempBuff == NULL)
            {
                char accumulatedList[10000] = {};
                bool hasActiveClients = false;
                for (int i = 0; i < connectionCount; i++)
                {
                    char locaList[1000] = {};
                    char localConcatList[1000] = {};
                    if (clientList[i].isActive)
                    {
                        hasActiveClients = true;
                        int ret = write(clientList[i].pipefd_p[1], token1, strlen(token1));
                        checkWrite(ret);
                        int readRet = read(clientList[i].pipefd_c[0], &locaList, 10000);
                        checkWrite(ret);
                        if (readRet > 0)
                        {
                            int ret = sprintf(localConcatList, "\t========== Client-ID: %s AND ClientIP: %s ==========\n%s\n \t\t\t===========================\n", clientList[i].client_id, clientList[i].ipAddress, locaList);
                            checkSprintf(ret);
                            strcat(accumulatedList, localConcatList);
                        }
                    }
                }
                if (hasActiveClients)
                {
                    int ret = write(STDOUT_FILENO, accumulatedList, strlen(accumulatedList));
                    checkWrite(ret);
                }
                else
                {
                    int ret = write(STDOUT_FILENO, "No Active Connections\n", strlen("No Active Connections\n"));
                    checkWrite(ret);
                }
            }
            else
            {
                char returnList[10000] = {};
                char cid[20];
                int ret = sprintf(cid, "%s", token);
                cid[ret - 1] = '\0';
                // write(STDOUT_FILENO, cid, strlen(cid));
                if (isValidClientId(cid))
                {
                    bool isClientActive = false;

                    // strcpy(cid, token);
                    for (int i = 0; i < connectionCount; i++)
                    {
                        if ((strcasecmp(clientList[i].client_id, cid) == 0) && clientList[i].isActive)
                        {
                            isClientActive = true;
                            int ret = write(clientList[i].pipefd_p[1], token1, strlen(token1));
                            checkWrite(ret);
                            int readRet = read(clientList[i].pipefd_c[0], &returnList, 10000);
                            checkRead(readRet);
                            if (readRet > 0)
                            {
                                char printResultList[10000];
                                int ret = sprintf(printResultList, "\t========== Client-ID: %s AND ClientIP: %s ==========\n%s\n \t\t\t===========================\n", clientList[i].client_id, clientList[i].ipAddress, returnList);
                                checkSprintf(ret);
                                int ret1 = write(STDOUT_FILENO, printResultList, ret);
                                checkWrite(ret1);
                                break;
                            }
                            break;
                        }
                    }
                    if (!isClientActive)
                    {
                        int ret = write(STDOUT_FILENO, "Client is not active\n", strlen("Client is not active\n"));
                        checkWrite(ret);
                    }
                }
                else
                {
                    int ret = write(STDOUT_FILENO, "Invalid Client ID\n", strlen("Invalid Client ID\n"));
                    checkWrite(ret);
                    continue;
                }
            }
        }
        else if (strcasecmp(token, "connlist") == 0)
        {
            displayClientList();
        }
        else
        {
            int ret = write(STDOUT_FILENO, "Invalid Command.Please Enter correct Command\n", strlen("Invalid Command.Please Enter correct Command\n"));
            checkWrite(ret);
        }
    }
}
//====================================  ********************** ==============================

//=========================  Thread: 2 Reading input from pipe in ClientHandler =============

/*
    Function: displayUSerInput
    Input:    void * param
    Output:        -
    Desc:     This Thread is created in ClientHandler and read constantly from pipe to perform operations.
*/

void *displayUSerInput(void *param)
{
    // char *context = NULL;
    char buff[100] = {};
    char token1[100] = {};
    char token2[100] = {};
    char token3[1000] = {};

    while (1)
    {
        char pipeBuff[100] = {};
        char modifiedBuff[100] = {};

        int readRet = read(clientList[connectionCount].pipefd_p[0], pipeBuff, 100);
        checkRead(readRet);
        // char chkMsg[1000];
        // sprintf(chkMsg, "In display:\t %s\n", pipeBuff);
        // write(STDOUT_FILENO, chkMsg, strlen(chkMsg));

        // char *token = strtok(pipeBuff, " ");
        char *token = strtok(pipeBuff, " ");

        if (token == NULL)
        {
            write(STDOUT_FILENO, "No command Passed. Enter Valid Command.\n", strlen("No command Passed. Enter Valid Command.\n"));
        }
        else if (strcasecmp(token, "print") == 0)
        {
            while (token != NULL)
            {

                token = strtok(NULL, "\n");
                if (token == NULL)
                {
                    int ret = write(STDOUT_FILENO, "No message given after print.\n", strlen("No message given after print.\n"));
                    checkWrite(ret);
                }
                else
                {
                    strcpy(modifiedBuff, token);
                    strcat(modifiedBuff, "\n");

                    int ret = write(clientList[connectionCount].sock, modifiedBuff, strlen(modifiedBuff));
                    checkWrite(ret);
                    ret = write(STDOUT_FILENO, "Message Sent Successfully!!\n", strlen("Message Sent Successfully!!\n"));
                    checkWrite(ret);
                }

                token = strtok(NULL, " ");
            }
        }
        else if (strcasecmp(token, "list") == 0)
        {
            printActivelist(proclist, clientList[connectionCount].pipefd_c[1]);
        }
        else if (strcasecmp(token, "listall") == 0)
        {
            printlist(proclist, clientList[connectionCount].pipefd_c[1]);
        }
    }
}
//====================================  ********************** ==============================

//====================================  Main Connection Function ============================

/*
    Function: connection
    Input:    void * sock
    Output:        -
    Desc:     This funct takes a scoket fd and accepts new connection, followed by forking -> creating ClientHandlers 
*/
void connection(int sock)
{
    // Declaring Variables
    int msgsock;                   // clientsocket fd
    struct sockaddr clientaddress; // Storing Client Information
    int address_len;               // Storing Address Length of ip.
    address_len = sizeof(clientaddress);
    char unique_id[200]; // Creating and Storing unique CLientHandler ID
    //thread variable

    int ret = sprintf(unique_id, "c%d", connectionCount);
    checkSprintf(ret);
    strcpy(clientList[connectionCount].client_id, unique_id);

    //Calling Signal Handler in Conn Process // will be inherit in child processes as well
    if (signal(SIGCHLD, signal_handler) == SIG_ERR)
    {
        perror("Cannot Handle SIGCHILd\n");
    }
    if (signal(SIGTERM, signal_handler) == SIG_ERR)
    {
        perror("Cannot Handle SIGCHILd\n");
    }
    if (signal(SIGINT, signal_handler) == SIG_ERR)
    {
        perror("Cannot Handle SIGCHILd\n");
    }
    msgsock = accept(sock, &clientaddress, &address_len);
    if (msgsock == -1)
        perror("accept");
    else
    {
        clientList[connectionCount].sock = msgsock;
        struct sockaddr_in *addr_in = (struct sockaddr_in *)&clientaddress;
        char *s = inet_ntoa(addr_in->sin_addr);
        int cport = (int)ntohs(addr_in->sin_port);
        strcpy(clientList[connectionCount].ipAddress, s);

        clientList[connectionCount].client_port = cport;
        clientList[connectionCount].isActive = true;

        // Initializing Pipes [Pipe fdfs usage describe below]
        /*
            pipefd_p[1] -> writes to pipe after reading from stdin in userInput Thread
            pipefd_p[0] -> reads from pipe in clientHandler Thread and performs operation.
            pipefd_c[1] -> writes to pipe after read end of other pipe to send list info to userThread.
            pipefd_c[0] -> reads from pipe in userInput Thread whatever data is written from clientHandler Thread
        */
        if (pipe(clientList[connectionCount].pipefd_p) == -1) //[1] write [0] read
        {
            perror("Failed to allocate pipes");
            exit(EXIT_FAILURE);
        }
        if (pipe(clientList[connectionCount].pipefd_c) == -1) // [1] CH write [1] [0] read
        {
            perror("Failed to allocate pipes");
            exit(EXIT_FAILURE);
        }

        int pid = fork();
        if (pid > 0) // Parent
        {
            close(msgsock);
            clientList[connectionCount].pid = pid;
            // close(clientList[connectionCount].pipefd_p[0]); //reading
            // close(clientList[connectionCount].pipefd_c[1]); //writing
            connectionCount++; // increementing connection count to intake more connections
        }

        if (pid == 0) //Client Handler
        {
            /*  
                In Child connection count will remain of that particluarclientHandler created, 
                it wont be increemented.
            */
            // close(clientList[connectionCount].pipefd_p[1]); //writing
            // close(clientList[connectionCount].pipefd_c[0]); //reading

            pthread_t thread2; //reading from pipe
            char pipeBuff[100];
            // struct client = (struct * ClientInformation) clientList[connectionCount];
            int iret2 = pthread_create(&thread2, NULL, &displayUSerInput, NULL);
            if (iret2)
            {
                fprintf(stderr, "Error - pthread_create() return code: %d\n", iret2);
                exit(EXIT_FAILURE);
            }

            bool isReading = true;
            bool isInvalidCommand = false;
            char buff[100];

            while (isReading)
            {
                int readSuccess = read(msgsock, &buff, sizeof(buff));
                checkRead(readSuccess);
                if (readSuccess == -1)
                {
                    exit(0);
                }
                // sleep(10);
                if (readSuccess == 0)
                {
                    for (int i = 0; i < sno; i++)
                    {
                        if (proclist[i].isActive)
                        {
                            int status;
                            int ret = kill(proclist[i].pid, SIGTERM);
                            if (ret == -1)
                            {
                                perror("Error while Killing");
                            }
                            else
                            {
                                proclist[i].endt = time(NULL);
                                proclist[i].isActive = false;
                            }
                        }
                    }
                    exit(0);
                }
                char temporaryBuff[100] = {};
                int ret = sprintf(temporaryBuff, "Recieved from Client: %s \n", buff);
                checkSprintf(ret);
                int writeRet = write(STDOUT_FILENO, temporaryBuff, ret);
                checkWrite(writeRet);
                char *token = strtok(buff, " ");

                if (token == NULL)
                {
                    int ret = write(msgsock, "No Command passed\n", strlen("No Command passed\n"));
                    checkWrite(ret);
                    continue;
                }
                else if (strcasecmp(token, "add") == 0)
                {
                    int sum = 0;
                    int acc = 0;
                    char sumBuff[100];
                    bool isValidInput = true;

                    token = strtok(NULL, " ");
                    if (token == NULL)
                    {
                        int ret = write(msgsock, "No List passed to add Numbers\n", strlen("No List passed to add Numbers\n"));
                        checkWrite(ret);
                    }
                    else
                    {
                        while (token != NULL)
                        {
                            if (validInput(token))
                            {
                                int s1 = sscanf(token, "%d", &acc);
                                if (s1 == -1)
                                {
                                    perror("error while writing to memory.");
                                }
                                else if (s1 == 1)
                                {
                                    sum += acc;
                                }
                            }
                            else
                            {
                                isValidInput = false;
                            }
                            token = strtok(NULL, " ");
                        }
                        if (isValidInput)
                        {
                            int ret = sprintf(sumBuff, "%d\n", sum);
                            if (ret == -1)
                            {
                                perror("Error while writing into memory");
                            }
                            else
                            {
                                int writeRet = write(msgsock, sumBuff, ret);
                            }
                        }
                        else
                        {
                            int writeRet = write(msgsock, "Cannot perform arithmatic operations on non-numeric list: Pass numeric List\n", strlen("Cannot perform arithmatic operations on non-numeric list: Pass numeric List\n"));
                            checkWrite(writeRet);
                        }
                    }
                }
                else if (strcasecmp(token, "sub") == 0)
                {
                    int sub = 0;
                    int acc = 0;
                    char subBuff[100];
                    bool isValidInput = true;

                    token = strtok(NULL, " ");
                    if (token == NULL)
                    {
                        int ret = write(msgsock, "No List passed to subtract Numbers\n", strlen("No List passed to subtract Numbers\n"));
                        checkWrite(ret);
                    }
                    else
                    {
                        int s1 = sscanf(token, "%d", &acc);

                        if (s1 == -1)
                        {
                            perror("error while writing to memory.");
                        }
                        else if (s1 == 1)
                        {
                            sub = acc;
                            token = strtok(NULL, " ");
                        }
                        while (token != NULL)
                        {
                            if (validInput(token))
                            {
                                int s1 = sscanf(token, "%d", &acc);
                                if (s1 == -1)
                                {
                                    perror("error while writing to memory.");
                                }
                                else if (s1 == 1)
                                {
                                    sub -= acc;
                                }
                            }
                            else
                            {
                                isValidInput = false;
                            }
                            token = strtok(NULL, " ");
                        }
                        if (isValidInput)
                        {
                            int ret = sprintf(subBuff, "%d\n", sub);
                            if (ret == -1)
                            {
                                perror("Error while writing into memory");
                            }
                            else
                            {
                                int writeRet = write(msgsock, subBuff, ret);
                                checkWrite(writeRet);
                            }
                        }
                        else
                        {
                            int writeRet = write(msgsock, "Cannot perform arithmatic operations on non-numeric list: Pass numeric List\n", strlen("Cannot perform arithmatic operations on non-numeric list: Pass numeric List\n"));
                            checkWrite(writeRet);
                        }
                    }
                }
                else if (strcasecmp(token, "mult") == 0)
                {
                    int prod = 1;
                    int acc = 0;
                    char prodBuff[100];
                    bool isValidInput = true;

                    token = strtok(NULL, " ");

                    if (token == NULL)
                    {
                        int ret = write(msgsock, "No List passed to multiply Numbers\n", strlen("No List passed to multiply Numbers\n"));
                        checkWrite(ret);
                    }
                    else
                    {
                        while (token != NULL)
                        {
                            if (validInput(token))
                            {
                                int s1 = sscanf(token, "%d", &acc);
                                if (s1 == -1)
                                {
                                    perror("error while writing to memory.");
                                }
                                else if (s1 == 1)
                                {
                                    prod *= acc;
                                }
                            }
                            else
                            {
                                isValidInput = false;
                            }
                            token = strtok(NULL, " ");
                        }
                        if (isValidInput)
                        {
                            int ret = sprintf(prodBuff, "%d\n", prod);
                            if (ret == -1)
                            {
                                perror("Error while writing into memory");
                            }
                            else
                            {
                                int writeRet = write(msgsock, prodBuff, ret);
                                checkWrite(writeRet);
                            }
                        }
                        else
                        {
                            int writeRet = write(msgsock, "Cannot perform arithmatic operations on non-numeric list: Pass numeric List\n", strlen("Cannot perform arithmatic operations on non-numeric list: Pass numeric List\n"));
                            checkWrite(writeRet);
                        }
                    }
                }
                else if (strcasecmp(token, "div") == 0) //token of div found
                {
                    float div = 1;
                    float acc;
                    char divBuff[100];
                    bool isValidInput = true;
                    bool hasDivisionByZeroOccured = false;

                    token = strtok(NULL, " ");

                    if (token == NULL)
                    {
                        int ret = write(msgsock, "No List passed to divide Numbers\n", strlen("No List passed to divide Numbers\n"));
                        checkWrite(ret);
                    }
                    else
                    {
                        int s1 = sscanf(token, "%f", &acc);

                        if (s1 == -1)
                        {
                            perror("error while writing to memory.");
                        }
                        else if (s1 == 1)
                        {
                            div = acc;
                            token = strtok(NULL, " ");
                        }
                        while (token != NULL)
                        {
                            if (strcmp(token, "0") == 0)
                            {
                                isValidInput = false;
                                hasDivisionByZeroOccured = true;
                                break;
                            }
                            if (validInput(token))
                            {
                                int s1 = sscanf(token, "%f", &acc);
                                if (s1 == -1)
                                {
                                    perror("error while writing to memory.");
                                }
                                else if (s1 == 1)
                                {
                                    div /= acc;
                                }
                            }
                            else
                            {
                                isValidInput = false;
                            }
                            token = strtok(NULL, " ");
                        }
                        if (isValidInput)
                        {
                            int ret = sprintf(divBuff, "%.2f\n", div);
                            if (ret == -1)
                            {
                                perror("Error while writing into memory");
                            }
                            else
                            {
                                int writeRet = write(msgsock, divBuff, ret);
                            }
                        }
                        else
                        {
                            if (hasDivisionByZeroOccured)
                            {
                                int writeRet = write(msgsock, "Division by Zero Occured.\n", strlen("Division by Zero Occured.\n"));
                                checkWrite(writeRet);
                            }
                            else
                            {
                                int writeRet = write(msgsock, "Cannot perform arithmatic operations on non-numeric list: Pass numeric List\n", strlen("Cannot perform arithmatic operations on non-numeric list: Pass numeric List\n"));
                                checkWrite(writeRet);
                            }
                        }
                    }
                }
                else if (strcasecmp(token, "list") == 0)
                {
                    token = strtok(NULL, " ");
                    if (token == 0)
                    {
                        printActivelist(proclist, msgsock);
                    }
                    else if (strcasecmp(token, "all") == 0)
                    {
                        token = strtok(NULL, " ");
                        if (token == NULL)
                        {
                            printlist(proclist, msgsock);
                        }
                        else
                        {
                            int ret = write(msgsock, "Invalid Token found after list all\n", strlen("Invalid Token found after list all\n"));
                            checkWrite(ret);
                        }
                    }
                    else
                    {
                        int ret = write(msgsock, "Invalid Token found after list\n", strlen("Invalid Token found after list\n"));
                        checkWrite(ret);
                    }
                }
                else if (strcasecmp(token, "run") == 0)
                {
                    token = strtok(NULL, " ");

                    if (token == NULL)
                    {
                        int retWrite = write(msgsock, "No process Name specified\n", strlen("No process Name specified\n"));
                        checkWrite(retWrite);
                    }
                    else
                    {
                        char procBuff[100];
                        strcpy(procBuff, token); //saving processName
                        token = strtok(NULL, " ");
                        if (token == NULL)
                        {
                            int pipe_process[2];
                            int pipRet = pipe2(pipe_process, O_CLOEXEC);
                            if (pipRet == -1)
                            {
                                perror("Failed to allocate pipes");
                                exit(EXIT_FAILURE);
                            }

                            int newCpid = fork();

                            if (newCpid == -1)
                            {
                                perror("Error while forking child from parent");
                            }

                            if (newCpid == 0) // chid process
                            {

                                // close(msgsock);
                                int exec_status = execlp(procBuff, procBuff, NULL);
                                // struct Process p_struct = {sno, token, true, time(0), '-'};

                                if (exec_status == -1)
                                {
                                    perror("Error occured while running Program");
                                }
                                int ret = write(pipe_process[1], "Exec Unsuccess\n", strlen("Exec Unsuccess\n"));
                                checkWrite(ret);
                                exit(0);
                            }
                            if (newCpid > 0)
                            {
                                close(pipe_process[1]);

                                char buffer[100] = {};

                                int readSuccess = read(pipe_process[0], buffer, 100);
                                if (readSuccess == -1)
                                {
                                    perror("Error read");
                                }
                                else if (readSuccess == 0) // exec passed
                                {
                                    char tempBuff[100];
                                    time_t t = time(NULL);
                                    proclist[sno].num = sno;

                                    proclist[sno].pid = newCpid;

                                    strcpy(proclist[sno].procName, procBuff);
                                    proclist[sno].isActive = true;
                                    proclist[sno].start = t;
                                    sno++;
                                    close(pipe_process[0]);
                                    int retWrite = write(msgsock, "Exec Passed\n", strlen("Exec Passed\n"));
                                    if (retWrite == -1)
                                    {
                                        perror("Error while displaying write error on line 448: ");
                                        exit(0);
                                    }

                                    // continue;
                                }
                                else // exec failed
                                {

                                    int retWrite = write(msgsock, "EXEC Failed\n", strlen("EXEC Failed\n"));
                                    checkWrite(retWrite);

                                    // exit(0);
                                }

                                close(pipe_process[0]);
                                // wait(NULL);
                            }
                        }
                        else
                        {
                            int retWrite = write(msgsock, "Invalid Token Passed after process Name\n", strlen("Invalid Token Passed after process Name\n"));
                            checkWrite(retWrite);
                        }
                    }
                }
                else if (strcasecmp(token, "kill") == 0)
                {
                    token = strtok(NULL, " ");
                    if (token == NULL)
                    {
                        int ret = write(msgsock, "No pid/name defined to kill process\n", strlen("No pid/name defined to kill process\n"));
                        checkWrite(ret);
                    }
                    else
                    {
                        char saveToken[100];
                        strcpy(saveToken, token);
                        token = strtok(NULL, " ");
                        if (token == NULL)
                        {
                            int pidToKill = atoi(saveToken);
                            time_t end_t;

                            if (pidToKill != 0)
                            {
                                bool isKill = false;
                                for (int i = 0; i < sno; i++)
                                {
                                    if (proclist[i].pid == pidToKill && proclist[i].isActive)
                                    {
                                        isKill = true;
                                        end_t = time(NULL);
                                        proclist[i].isActive = false;
                                        proclist[i].endt = end_t;
                                        int killRet = kill(atoi(saveToken), SIGTERM);
                                        if (killRet == -1)
                                        {
                                            perror("Error while Killing");
                                        }
                                        int ret = write(msgsock, "Process Terminated Successfully\n", strlen("Process Terminated Successfully\n"));
                                        checkWrite(ret);
                                    }
                                }
                                if (!isKill)
                                {
                                    int ret = write(msgsock, "Process with given pid does not exist\n", strlen("Process with given pid does not exist\n"));
                                    checkWrite(ret);
                                }
                            }
                            else
                            {
                                bool isFound = false;
                                for (int i = 0; i < sno; i++)
                                {
                                    if ((strcmp(proclist[i].procName, saveToken) == 0) && proclist[i].isActive)
                                    {
                                        isFound = true;
                                        end_t = time(NULL);
                                        proclist[i].isActive = false;
                                        proclist[i].endt = end_t;
                                        int retKill = kill(proclist[i].pid, SIGTERM);
                                        if (retKill == -1)
                                        {
                                            perror("Error kill");
                                        }
                                        int ret = write(msgsock, "Process Terminated Successfully\n", strlen("Process Terminated Successfully\n"));
                                        checkWrite(ret);
                                        break;
                                    }
                                }
                                if (!isFound)
                                {
                                    int ret = write(msgsock, "You dont have that process in your list.\n", strlen("You dont have that process in your list.\n"));
                                    checkWrite(ret);
                                }
                            }
                        }
                        else
                        {
                            int retWrite = write(msgsock, "Invalid Token Passed after Kill <pid/Name>\n", strlen("Invalid Token Passed after Kill <pid/Name>\n"));
                            checkWrite(retWrite);
                        }
                    }
                }
                else if (strcasecmp(token, "exit") == 0)
                {
                    signal(SIGCHLD, SIG_DFL);
                    pid_t w;
                    int wstatus;
                    isReading = false;
                    for (int i = 0; i < sno; i++)
                    {
                        if (proclist[i].isActive)
                        {
                            int status;
                            int ret = kill(proclist[i].pid, SIGTERM);
                            if (ret == -1)
                            {
                                perror("Error while Killing");
                            }
                            else
                            {
                                proclist[i].endt = time(NULL);
                                proclist[i].isActive = false;
                            }
                        }
                    }

                    int ret = write(msgsock, "exit", strlen("exit"));
                    checkWrite(ret);
                    // close(msgsock);
                    exit(EXIT_SUCCESS);
                }
                else
                {
                    isInvalidCommand = true;
                    int ret = write(msgsock, "Invalid Command\n", strlen("Invalid Command\n"));
                    checkWrite(ret);
                }
            }

            close(msgsock);
        }

        // close(msgsock);
    }
}

/*
 * This program creates a socket and then begins an infinite loop. Each time
 * through the loop it accepts a connection and prints out messages from it. 
 * When the connection breaks, or a termination message comes through, the
 * program accepts a new connection. 
 */

int main(void)
{
    int sock, length;
    struct sockaddr_in server;
    pthread_t inputThread;
    char buf[1024];
    int rval;
    int i;

    /* Create socket */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("opening stream socket");
        exit(1);
    }
    int option = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    /* Name socket using wildcards */
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(SERVER_PORT);
    // htons(0)
    if (bind(sock, (struct sockaddr *)&server, sizeof(server)))
    {
        perror("binding stream socket");
        exit(1);
    }
    /* Find out assigned port number and print it out */
    length = sizeof(server);
    if (getsockname(sock, (struct sockaddr *)&server, (socklen_t *)&length))
    {
        perror("getting socket name");
        exit(1);
    }
    printf("Socket has port #%d\n", ntohs(server.sin_port));
    fflush(stdout);

    //Thread for taking input from user
    int iret1 = pthread_create(&inputThread, NULL, &takeuserInput, NULL);
    if (iret1)
    {
        fprintf(stderr, "Error - pthread_create() return code: %d\n", iret1);
        exit(EXIT_FAILURE);
    }

    /* Start accepting connections */
    listen(sock, 5);

    do
    {
        char msock[200];
        connection(sock);

    } while (true);
    /*
	 * Since this program has an infinite loop, the socket "sock" is
	 * never explicitly closed.  However, all sockets will be closed
	 * automatically when a process is killed or terminates normally. 
	 */
}
