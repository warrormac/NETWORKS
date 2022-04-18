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

//#define PORT 45011            /* port */
//#define SERVER_ADDRESS "5.253.235.219" /* IP, only IPV4 support  */




inline string zeros(int num, int digits){
    string strNum = to_string(num);
    return string(digits - strNum.size(),'0') + strNum;
}

bool isDirectMsg(string msg){
    std::size_t found = msg.find(',');
    if (found!=std::string::npos) return 1;
    return 0;
}


void splitNickMsg(string txt ,string& nick , string& msg){
    string delimiter = ",";
    nick = txt.substr(0,txt.find(delimiter));
    nick.erase(remove(nick.begin(),nick.end(),' '),nick.end());
    msg = txt.substr(txt.find(delimiter)+1,txt.size());
}


void READ(int sockfd){

    char buff_rx[1010];

    for(;;){

        bzero(buff_rx,1010); //clean buffer
        read(sockfd , buff_rx , 4); // read action and size

        if(buff_rx[0] == 'M'){
            int size = atoi(&buff_rx[1]);
            bzero(buff_rx,1010); //clean buffer
            read(sockfd,buff_rx,size);
            cout<<"\n "<<buff_rx<<" \n";
//            printf("\n[ SERVER ] : %s\n" ,buff_rx);
        }
        if(buff_rx[0] == 'Q'){
            printf("\n server left the chat \n");
            break;
        }
    }

    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);

    cout << "\nRead_thread termino.\n";
}



void WRITE(int sockfd){

    string buff_tx;
    string nickname;

    cout<<"\nEnter your nickname > ";
    getline(cin,nickname);


    buff_tx = "N" + zeros(nickname.size(),3) + nickname;
    int n = write(sockfd, buff_tx.c_str(), buff_tx.size());
//    cout << "\nProtocolo:" << buff_tx<<endl ;
    if (n < 0) perror("ERROR writing to server");


    do{

        //get message

        getline(cin,buff_tx);

        //Quit Action
        if(buff_tx == "Q"){
            buff_tx += "000" ;
//            cout << "\nProtocolo:" << buff_tx<<endl ;
            write(sockfd,buff_tx.c_str(),4);
            break;
        }

        //Direct Message
        if(isDirectMsg(buff_tx)){
            string nick , msg;
            splitNickMsg(buff_tx,nick,msg);
            buff_tx = "D" + zeros(msg.size(),3) + msg + zeros(nick.size(),2) + nick ;
            cout<<"\nenviaste: "<<buff_tx<<"\n";
        }else{
            //write message
            buff_tx = "M" + zeros(buff_tx.size(),3) + buff_tx;
        }

        int n = write(sockfd, buff_tx.c_str(), buff_tx.size());
        if(n < 0) perror("ERROR writing to server");
//        cout << "\nProtocolo:" << buff_tx ;

    }while(true);

    shutdown(sockfd,SHUT_RDWR);
    close(sockfd);
    cout << "\nWrite_thread termino.\n";

}







/* This clients connects, sends a text and disconnects */
int main()
{
    int sockfd;
    struct sockaddr_in servaddr;

    /* Socket creation */
    sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
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

    return 0;
}
