/*************************************************************************************/
/* @file    client_1.c                                                               */
/* @brief   This clients connects,                                                   */
/*          sends a text, reads what server and disconnects                          */
/*************************************************************************************/

// CPP
#include <iostream>
#include <string>
#include <bits/stdc++.h> //stl
using namespace std;

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>


/*Multi threading*/
#include <thread>

#define SERVER_ADDRESS "127.0.0.1" /* server IP */
#define PORT 8080  

/* Test sequences */
char buf_tx[] = "Hello server. I am a client";
char buf_rx[100]; /* receive buffer */

inline string zeros(int num){
    string strNum = to_string(num);
    return string(3 - strNum.size(),'0') + strNum;
}


void READ(int sockfd){

    char buff_rx[1010];

    for (;;) {

        bzero(buff_rx,1010); //clean buffer
        read(sockfd , buff_rx , 4); // read action and size

        if(buff_rx[0] == 'M'){
            int size = atoi(&buff_rx[1]);
            bzero(buff_rx,1010); //clean buffer
            read(sockfd,buff_rx,size);
            printf("\n[ SERVER ] : %s" ,buff_rx);
        }
        if(buff_rx[0] == 'Q'){
            printf("\n server left the chat ");
            // receptions and transmissions will be disallowed.
            shutdown(sockfd , SHUT_RDWR);
            close(sockfd);
            break;
        }

    }

}



void WRITE(int sockfd){


    string buff_tx;
    string nickname;

    cout<<"\nEnter your nickname > ";
    getline(cin,nickname);

    buff_tx = "N" + zeros(nickname.size()) + nickname;
    int n = write(sockfd, buff_tx.c_str(), buff_tx.size());
    if (n < 0) perror("ERROR writing to server");

    for(;;)
    {

        //get message
        cout<<"\n[ "<<nickname<<" ] > ";
        getline(cin,buff_tx);

        //Quit Action
        if(buff_tx == "Q"){
            // receptions and transmissions will be disallowed
            shutdown(sockfd, SHUT_RDWR);
            close(sockfd);
            break;
        }

        //write message
        buff_tx = "M" + zeros(buff_tx.size()) + buff_tx;
        int n = write(sockfd, buff_tx.c_str(), buff_tx.size());
        if(n < 0) perror("ERROR writing to server");

    }


}







/* This clients connects, sends a text and disconnects */
int main()
{
    int sockfd;
    struct sockaddr_in servaddr;

    /* Socket creation */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        printf("CLIENT: socket creation failed...\n");
        return -1;
    }
    else
    {
        printf("CLIENT: Socket successfully created..\n");
    }

    // clear socket
    memset(&servaddr, 0, sizeof(servaddr));

    /* assign IP, PORT */
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
    servaddr.sin_port = htons(PORT);

    /* try to connect the client socket to server socket */
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0)
    {
        printf("connection with the server failed...\n");
        return -1;
    }

    printf("connected to the server..\n");




    thread th1(READ, sockfd);
    thread th2(WRITE, sockfd);

    th1.join();
    th2.join();


    /* close the socket */
    close(sockfd);
}
