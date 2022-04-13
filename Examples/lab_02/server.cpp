//
// Created by misash on 10/04/22.
//


/* Server code in C */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <iostream>

using namespace std;

int q=0;
string nick="Reply: ";
void *leer(void *SocketFD)
{
    while (q==0)
    {
        int n;
        char buffer[1010];
        int *SocketFD1=(int* )SocketFD;

        n = read(*SocketFD1,buffer,1010);
        if (n < 0) perror("ERROR reading from socket");
        cout<<nick<<": "<<buffer<<"\n";

        bzero(buffer,1010);

    }

}

void *esc(void *SocketFD)
{
    while (q==0)
    {


        int n;
        char *temp;
        int *SocketFD1=(int* )SocketFD;
        int tam=0;
        string x;


        getline(cin, x);
        tam=x.length();


        char buffer[tam+1];
        strcpy(buffer, x.c_str());


        if (buffer[0]=='q' && tam==1)
        {
            cout<<"Session Terminated \n";
            n = write(*SocketFD1,"Usuario se retiro\n",255);
            break;
        }

        if (buffer[0]=='n' && tam==1)
        {
            getline(cin, nick);
            x="";
            strcpy(buffer, x.c_str());

        }

        n = write(*SocketFD1,buffer,255);
        if (n < 0) perror("ERROR reading from socket");
        cout<<nick<<": "<<buffer<<"\n";
        x="";
    }

}


int main(void)
{
    struct sockaddr_in stSockAddr;
    int SocketServer = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    char buffer[1010];

    if(-1 == SocketServer)
    {
        perror("can not create socket");
        exit(EXIT_FAILURE);
    }

    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));

    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(8000);
    stSockAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if(-1 == bind(SocketServer,(const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in)))
    {
        perror("error bind failed");
        close(SocketServer);
        exit(EXIT_FAILURE);
    }

    if(-1 == listen(SocketServer, 10))
    {
        perror("error listen failed");
        close(SocketServer);
        exit(EXIT_FAILURE);
    }

    for(;;)
    {
        int SocketFD = accept(SocketServer, NULL, NULL);

        if(0 > SocketFD)
        {
            perror("error accept failed");
            close(SocketServer);
            exit(EXIT_FAILURE);
        }


        pthread_t thread_id[2];

        pthread_create(&thread_id[1],NULL, leer,(&SocketFD ));

        pthread_create(&thread_id[2],NULL, esc,(&SocketFD ));

        pthread_join(thread_id[1],NULL);
        pthread_join(thread_id[2],NULL);



        /* perform read write operations ... */

        shutdown(SocketFD, SHUT_RDWR);
        close(SocketFD);
    }

    close(SocketServer);
    return 0;
}