//
// Created by misash on 10/04/22.
//

/* Client code in Cpp */

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
        char buffer[256];
        int *SocketFD1=(int* )SocketFD;

        n = read(*SocketFD1,buffer,255);
        if (n < 0) perror("ERROR reading from socket");
        cout<<nick<<": "<<buffer<<"\n";

        bzero(buffer,256);

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

int main(int argc, char *argv[])
{
    struct sockaddr_in stSockAddr;
    int Res;
    int SocketFD = socket(AF_INET, SOCK_STREAM, 0); //IPPROTO_TCP
    int n;

    if (-1 == SocketFD)
    {
        perror("cannot create socket");
        exit(EXIT_FAILURE);
    }



    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));

    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(8000);
    Res = inet_pton(AF_INET, "127.0.0.1", &stSockAddr.sin_addr); //IP Addres

    if (0 > Res)
    {
        perror("error: first parameter is not a valid address family");
        close(SocketFD);
        exit(EXIT_FAILURE);
    }
    else if (0 == Res)
    {
        perror("char string (second parameter does not contain valid ipaddress");
        close(SocketFD);
        exit(EXIT_FAILURE);
    }

    if (-1 == connect(SocketFD, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in)))
    {
        perror("connect failed");
        close(SocketFD);
        exit(EXIT_FAILURE);
    }






    pthread_t thread_id[2];



    pthread_create(&thread_id[2],NULL, esc,(&SocketFD ));
    pthread_create(&thread_id[1],NULL, leer,(&SocketFD ));
    pthread_join(thread_id[1],NULL);
    pthread_join(thread_id[2],NULL);



    shutdown(SocketFD, SHUT_RDWR);

    close(SocketFD);
    return 0;
}