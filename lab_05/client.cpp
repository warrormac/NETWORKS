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
#include <fstream>

/*Multi threading*/
#include <thread>

//#define SERVER_ADDRESS "127.0.0.1" /* server IP */
//#define PORT 8000

//#define PORT 45012            /* port */
#define SERVER_ADDRESS "5.253.235.219" /* IP, only IPV4 support  */




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



void splitNickFile(string buff,string& nick,string& fileName){

    buff.erase(remove(buff.begin(),buff.end(),' '),buff.end());
    nick = buff.substr(buff.find("(")+1,buff.find(",") - buff.find("(") -1);
    fileName = buff.substr(buff.find(',')+1,buff.find(')') - buff.find(',')-1);

}


bool isSendFile(string buff){
    buff.erase(remove(buff.begin(),buff.end(),' '),buff.end());
    return buff.size()> 6 && buff[0] == 'F' && buff[1] == '(' && buff.find(',') !=std::string::npos && buff[buff.size()-1] == ')';
}


void READ(int sockfd){

    char buff_rx[1010];

    for(;;){

        bzero(buff_rx,1010); //clean buffer
        read(sockfd , buff_rx , 4); // read action and size

        if(buff_rx[0] == 'M' ){
            int size = atoi(&buff_rx[1]);
            bzero(buff_rx,1010); //clean buffer
            read(sockfd,buff_rx,size);
            cout<<"\n "<<buff_rx<<" \n";
//            printf("\n[ SERVER ] : %s\n" ,buff_rx);
        }
        else if(buff_rx[0] == 'F'){
            //read file Name
            string fileName;
            int size = atoi(&buff_rx[1]);
            bzero(buff_rx,1000); //clean buffer
            read(sockfd,buff_rx,size);
            fileName = buff_rx;

            //readNick
            string nick;
            bzero(buff_rx,1010); //clean buffer
            read(sockfd,buff_rx,2);
            size = atoi(&buff_rx[0]);
            bzero(buff_rx,1010); //clean buffer
            read(sockfd,buff_rx,size);
            nick = buff_rx;


            //readfile Size
            bzero(buff_rx,1000); //clean buffer
            read(sockfd,buff_rx,9);
            int file_size = atoi(&buff_rx[0]);


            //read bytes
            char *buffer;
            int SIZE = 1024;
            fileName = "./recibidos/" + fileName;

            ofstream file;
            file.open(fileName , ios::out);

            while ( file_size) {

                if (file_size < SIZE) {
                    SIZE = file_size;
                }

                buffer = new char[SIZE];

                if (read(sockfd, buffer, SIZE) > 0) {
                    file.write(buffer,SIZE);
                }
                file_size -= SIZE;
            }
            string msg = "\n[ " + nick + " ] <private>:  send you the file : (" + fileName +")\n" ;
            cout<<msg;

            file.close();

        }
        else if(buff_rx[0] == 'Q'){
            printf("\n server left the chat \n");
            break;
        }
        else if(buff_rx[0] == 'E'){
            int size = atoi(&buff_rx[1]);
            bzero(buff_rx,1010); //clean buffer
            read(sockfd,buff_rx,size);
            cout<<"\n "<<buff_rx<<" \n";
        }
        else if(buff_rx[0] == 'l'){
            string nick ;
            int nickSize ;
            int size = atoi(&buff_rx[1]);
            cout<<"\n User List: \n";
            while(size--) {
                //read nickname size
                bzero(buff_rx,1010); //clean buffer
                read(sockfd,buff_rx,2);
                int nickSize = atoi(&buff_rx[0]);

                //read nickname
                bzero(buff_rx,1010); //clean buffer
                read(sockfd,buff_rx,nickSize);
                nick = buff_rx;
                cout<<"\t-> "<<nick<<" \n";
            }
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
//    cin.ignore();

    buff_tx = "N" + zeros(nickname.size(),3) + nickname;
    int n = write(sockfd, buff_tx.c_str(), buff_tx.size());
    if (n < 0) perror("ERROR writing to server");


    do{

        //get message

        getline(cin,buff_tx);

        //Quit Action
        if(buff_tx == "Q"){
            buff_tx += "000" ;
            write(sockfd,buff_tx.c_str(),4);
            break;
        }


        if(isSendFile(buff_tx)) {

            string nick, fileName;
            splitNickFile(buff_tx, nick, fileName);

            //write
            ifstream file;
            file.open(fileName, ios::out);

            //get size
            file.seekg(0, ios::end);
            int file_size = file.tellg();
            string fileSize = std::to_string(file_size);


            //send initial protocol details
            string protocol = "F" + zeros(fileName.size(),3) + fileName + zeros(nick.size(),2) + nick + zeros(file_size,9) ;
            write(sockfd, protocol.c_str(), protocol.size());

//            cout<<"\n client size: "<<file_size<<"\n";
//            cout<<"\n client sizestring: "<< zeros(fileSize.size(),9)<<"\n";

            //read bytes
            file.seekg(0, ios::beg);
            int bytes_read;
            char *buffer;
            int SIZE = 1024;
            int connfd;

            while (!file.eof() && file_size) {

                if (file_size < SIZE) {
                    SIZE = file_size;
                }

                buffer = new char[SIZE];
                if (file.read( buffer, SIZE)) {
                    write(sockfd, buffer, SIZE);
                }

                file_size -= SIZE;
            }

            file.close();

        }
        else{

            if(isDirectMsg(buff_tx)){
                //Direct Message
                string nick , msg;
                splitNickMsg(buff_tx,nick,msg);
                buff_tx = "D" + zeros(msg.size(),3) + msg + zeros(nick.size(),2) + nick ;
            }else if( buff_tx == "L"){
                //list clients request
                buff_tx = "L000";
            }
            else{
                //write message
                buff_tx = "M" + zeros(buff_tx.size(),3) + buff_tx;
            }
            int n = write(sockfd, buff_tx.c_str(), buff_tx.size());
            if(n < 0) perror("ERROR writing to server");
        }

    }while(true);

    shutdown(sockfd,SHUT_RDWR);
    close(sockfd);
    cout << "\nWrite_thread termino.\n";

}


int main()
{
    int sockfd;
    int PORT=0;
    struct sockaddr_in servaddr;
    cout<<"ingrese Puerto: ";
    cin>>PORT;
    cin.ignore();
//    / Socket creation /
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

//    / assign IP, PORT /
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
    servaddr.sin_port = htons(PORT);

//    / try to connect the client socket to server socket /
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




//
///* This clients connects, sends a text and disconnects */
//int main()
//{
//    int sockfd;
//    struct sockaddr_in servaddr;
//
//    /* Socket creation */
//    sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
//    if (sockfd == -1)
//    {
//        printf("CLIENT: socket creation failed...\n");
//        return -1;
//    }
//    else
//    {
//        printf("CLIENT: Socket successfully created..\n");
//    }
//
//    // clear socket
//    memset(&servaddr, 0, sizeof(servaddr));
//
//    /* assign IP, PORT */
//    servaddr.sin_family = AF_INET;
//    servaddr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
//    servaddr.sin_port = htons(PORT);
//
//    /* try to connect the client socket to server socket */
//    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0)
//    {
//        printf("connection with the server failed...\n");
//        return -1;
//    }
//
//    printf("connected to the server..\n");
//
//
//
//    thread th1(READ, sockfd);
//    thread th2(WRITE, sockfd);
//
//    th1.join();
//    th2.join();
//
//    return 0;
//}
