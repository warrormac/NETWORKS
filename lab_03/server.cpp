
#include <iostream>

/*standard symbols */
#include <unistd.h>

/* sockets */
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

/* strings / errors*/
#include <errno.h>
#include <stdio.h>
#include <string.h>

/* server parameters */
#define SERV_PORT 8080             /* port */
#define SERV_HOST_ADDR "127.0.0.1" /* IP, only IPV4 support  */
//#define SERV_PORT 45011            /* port */
//#define SERV_HOST_ADDR "5.253.235.219" /* IP, only IPV4 support  */
#define BUF_SIZE 100               /* Buffer rx, tx max size  */
#define BACKLOG 10                  /* Max. client pending connections  */

/*Multi threading*/
#include <thread>
#include<map>
#include <bits/stdc++.h>

using namespace std;


map<int , string> room;

void broadcast(string msg){
    int n;
    char tamano[100];
    for (map<int,string>::iterator it=room.begin(); it!=room.end(); ++it){
        string a = "M";
        sprintf(tamano,"%03d",msg.size());
        tamano[4]='\0';
        string t = tamano;
        string buffer = a + t + msg ;
        n = write(it->first, buffer.c_str(), buffer.size());
        cout << "Protocolo:" << buffer << endl;
    }
}
bool isSecretMsg(string msg){
    std::size_t found = msg.find(',');
    if (found!=std::string::npos) return 1;
    return 0;
}

void sendMsgByNick(string nick , string msg){
    int n;
    char tamano[100];
    for (auto it=room.begin(); it!=room.end(); ++it){
        if(it->second == nick){
            string a = "M";
            sprintf(tamano,"%03d",msg.size());
            tamano[4]='\0';
            string t = tamano;
            string buffer = a + t + msg ;
            n = write(it->first, buffer.c_str(), buffer.size());
            cout << "Secret Protocolo:" << buffer << endl;
        }
    }
}


string decodeMsg(string txt){
    string delimiter = ",";
    string nick = txt.substr(0,txt.find(delimiter));
    nick.erase(remove(nick.begin(),nick.end(),' '),nick.end());
    string msg = txt.substr(txt.find(delimiter)+1,txt.size());
    msg = "\n[ " + nick + " ] <private>: " + msg + "\n" ;
    sendMsgByNick(nick , msg);
    return msg;
}



inline string zeros(int num){
    string strNum = to_string(num);
    return string(3 - strNum.size(),'0') + strNum;
}

void READ(int connfd)
{

    char buff_rx[1010];
    char nickname[1000];
    int n ;
    string message;

    for(;;)
    {
        bzero(buff_rx,1010); //clean buffer
        read(connfd , buff_rx , 4); // read action and size

        if(buff_rx[0] == 'N'){
            int size = atoi(&buff_rx[1]);
            bzero(nickname,1000); //clean buffer
            read(connfd,nickname,size);
            room[connfd] = nickname;    //add client
            message = "\n " + room[connfd] + " enter the chat\n";
            cout<<message;
            broadcast(message);
        }
        else if(buff_rx[0] == 'M'){

            int size = atoi(&buff_rx[1]);
            bzero(buff_rx,1010); //clean buffer
            read(connfd,buff_rx,size);
            if(isSecretMsg(buff_rx)){
                message = decodeMsg(buff_rx);
            }else{
                message = "\n[" + room[connfd] + " ] : " + buff_rx + "\n" ;
                broadcast(message);
            }
            cout<<message;

        }
        else if(buff_rx[0] == 'Q'){
            message = "\n" + room[connfd] + "left the chat\n";
            cout<<message;
            write(connfd,"Q000",4);
            broadcast(message);
            break;
        }

    }

    // receptions and transmissions will be disallowed.
    shutdown(connfd , SHUT_RDWR);
    close(connfd);
    cout << "Read_thread termino.\n";
}


void WRITE(int connfd){

    do{
        string buff_tx;

        //send message
        getline(cin,buff_tx);

        if(buff_tx == "Q"  ){
            buff_tx += "000" ;
//            cout << "\nProtocolo:" << buff_tx <<endl;
            write(connfd,buff_tx.c_str(),4);
            break;
        }

        //write message
        buff_tx = "M" + zeros(buff_tx.size())  + buff_tx ;
        int n = write(connfd, buff_tx.c_str(), buff_tx.size());
//        cout << "\nProtocolo:" << buff_tx <<endl;
        if(n < 0) perror("\nERROR writing to client");

    }while(true);

    // receptions will be disallowed
    shutdown(connfd, SHUT_RDWR);
    close(connfd);

}




int main() /* input arguments are not used */
{
    int sockfd, connfd; /* listening socket and connection socket file descriptors */
    unsigned int len;   /* length of client address */
    struct sockaddr_in servaddr, client;



    /* socket creation */
    sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == -1)
    {
        fprintf(stderr, "[SERVER-error]: socket creation failed. %d: %s \n", errno, strerror(errno));
        return -1;
    }
    else
    {
        printf("[SERVER]: Socket successfully created..\n");
    }

    /* clear structure */
    memset(&servaddr, 0, sizeof(servaddr));

    /* assign IP, SERV_PORT, IPV4 */
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(SERV_PORT);

    /* Bind socket */
    if ((bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
    {
        fprintf(stderr, "[SERVER-error]: socket bind failed. %d: %s \n", errno, strerror(errno));
        return -1;
    }
    else
    {
        printf("[SERVER]: Socket successfully binded \n");
    }

    /* Listen */
    if ((listen(sockfd, 10)) != 0)
    {
        fprintf(stderr, "[SERVER-error]: socket listen failed. %d: %s \n", errno, strerror(errno));
        return -1;
    }
    else
    {
        printf("[SERVER]: Listening on SERV_PORT %d \n\n", ntohs(servaddr.sin_port));
    }


    /* Accept the data from incoming sockets in a iterative way */
    while (1)
    {
        connfd = accept(sockfd, NULL, NULL);
        if (connfd < 0)
        {
            fprintf(stderr, "[SERVER-error]: connection not accepted. %d: %s \n", errno, strerror(errno));
            return -1;
        }
        else
        {
            thread (READ, connfd).detach();
            thread (WRITE, connfd).detach();

        }

    }
    close(sockfd);

    return 0;
}
