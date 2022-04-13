/* Server code in C */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
using namespace std;
//
string fill0(int size){
    string str = to_string(size);
    str = string(3-str.size(),'0')+str;
    return str;
}
int main(void) {
    bool on = true;
    struct sockaddr_in stSockAddr;
    int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    //char buffer[1003],nickname[1000];
    int n;

    if (-1 == SocketFD) {
        perror("can not create socket");
        exit(EXIT_FAILURE);
    }

    memset( & stSockAddr, 0, sizeof(struct sockaddr_in));

    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(8080); //Puerto del server
    stSockAddr.sin_addr.s_addr = inet_addr("127.0.0.1") ;

    if (-1 == bind(SocketFD, (const struct sockaddr * ) & stSockAddr, sizeof(struct sockaddr_in))) {
        perror("error bind failed");
        close(SocketFD);
        exit(EXIT_FAILURE);
    }

    if (-1 == listen(SocketFD, 10)) {
        perror("error listen failed");
        close(SocketFD);
        exit(EXIT_FAILURE);
    }

    int ConnectFD = accept(SocketFD, NULL, NULL);

    if (0 > ConnectFD) {
        perror("error accept failed");
        close(SocketFD);
        exit(EXIT_FAILURE);
    }

    char nickname[1000]="\0";

    int pid;
    pid = fork();
    if (pid == 0) { //hijo
        //READ
        char buffer[1003];
        while (on) {
            bzero(buffer, 1003); //clean buffer
            read(ConnectFD, buffer, 4);
            if (buffer[0] == 'N') {
                int size = atoi( & buffer[1]);
                bzero(nickname, 1000); //clean nickname
                read(ConnectFD, nickname, size);
                cout<<"El nickname recibido es "<<nickname<<endl;
            }
            if (buffer[0] == 'M') {
                int size = atoi( & buffer[1]);
                read(ConnectFD, & buffer[4], size);
                printf("%s: [%s]\n", nickname, &buffer[4]);
            }
            if (buffer[0] == 'Q') {
                cout<<nickname<<" abandono la conversacion"<<endl;
                shutdown(ConnectFD, SHUT_RDWR);
                close(ConnectFD);
                close(SocketFD);
                on=false;
            }
        }
    }
    else { //Padre
        //WRITE
        do{
            string buffer;
            getline(cin, buffer);
            if(buffer=="Q"){
                on=false;
                shutdown(SocketFD, SHUT_RDWR);
                close(SocketFD);
                break;
            }
            int size = buffer.size();
            buffer = "M"+fill0(size)+buffer;
            int n = write(ConnectFD, buffer.c_str(), buffer.size());
            if (n < 0) perror("ERROR writing to socket");
        } while(on);
    }

    return 0;
}
