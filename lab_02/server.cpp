
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
#define BUF_SIZE 100               /* Buffer rx, tx max size  */
#define BACKLOG 5                  /* Max. client pending connections  */

/*Multi threading*/
#include <thread>

using namespace std;


inline string zeros(int num){
    string strNum = to_string(num);
    return string(3 - strNum.size(),'0') + strNum;
}

void READ(int connfd)
{

    char buff_rx[1010];
    char nickname[1000];

    for(;;)
    {
        bzero(buff_rx,1010); //clean buffer
        read(connfd , buff_rx , 4); // read action and size

        if(buff_rx[0] == 'N'){
            int size = atoi(&buff_rx[1]);
            bzero(nickname,1000); //clean buffer
            read(connfd,nickname,size);
            printf("\n %s enter the chat " ,nickname);
        }
        if(buff_rx[0] == 'M'){
            int size = atoi(&buff_rx[1]);
            bzero(buff_rx,1010); //clean buffer
            read(connfd,buff_rx,size);
            printf("\n[ %s ] : %s" ,nickname,buff_rx);
        }
        if(buff_rx[0] == 'Q'){
            printf("\n %s  left the chat " ,nickname );
            // receptions and transmissions will be disallowed.
            shutdown(connfd , SHUT_RDWR);
            close(connfd);
            break;
        }

    }
    
}


void WRITE(int connfd){

    for(;;)
    {
        string buff_tx;


        cout << "\n[server] >";
        getline(cin,buff_tx);

        if(buff_tx == "Q"  ){
            // receptions will be disallowed
            shutdown(connfd, SHUT_RD);
            close(connfd);
            break;
        }

        //write message
        buff_tx = "M" + zeros(buff_tx.size()) + buff_tx ;
        int n = write(connfd, buff_tx.c_str(), buff_tx.size());
        if(n < 0) perror("ERROR writing to client");

    }

}







int main() /* input arguments are not used */
{
    int sockfd, connfd; /* listening socket and connection socket file descriptors */
    unsigned int len;   /* length of client address */
    struct sockaddr_in servaddr, client;

    int len_rx, len_tx = 0;                                   /* received and sent length, in bytes just initialize */
    char buff_tx[BUF_SIZE] = "Hello client, I am the server"; /* message to client*/
    char buff_rx[BUF_SIZE];                                   /* buffers for reception  */

    /* socket creation */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
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
    servaddr.sin_addr.s_addr = inet_addr(SERV_HOST_ADDR);
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
    if ((listen(sockfd, BACKLOG)) != 0)
    {
        fprintf(stderr, "[SERVER-error]: socket listen failed. %d: %s \n", errno, strerror(errno));
        return -1;
    }
    else
    {
        printf("[SERVER]: Listening on SERV_PORT %d \n\n", ntohs(servaddr.sin_port));
    }

    len = sizeof(client);

    /* Accept the data from incoming sockets in a iterative way */
    while (1)
    {
        connfd = accept(sockfd, (struct sockaddr *)&client, &len);
        if (connfd < 0)
        {
            fprintf(stderr, "[SERVER-error]: connection not accepted. %d: %s \n", errno, strerror(errno));
            return -1;
        }
        else
        {
            thread th1(READ, connfd);
            thread th2(WRITE, connfd);
            th1.join();
            th2.join();
        }
    }

    close(sockfd);
}
