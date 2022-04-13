 /* Client code in C */
 
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <unistd.h>
 
 #include <iostream>       // std::cout
 #include <thread> 
 #include <string>
    
using namespace  std;


void read_thread(int socket_cliente) {
  char buffer[1000];
  char accion;
  int n, tamano;
  do{
    n = read(socket_cliente,buffer,4);
    accion = buffer[0];
    buffer[0]='0';
    buffer[4]='\0';
    tamano = atoi(buffer);
    if (accion == 'N'){
      n = read(socket_cliente,buffer,tamano);
      buffer[n]='\0';
      cout << buffer << ":";
    }
    else  if (accion == 'M'){
      n = read(socket_cliente,buffer,tamano);
      buffer[n]='\0';
      cout << buffer << "\n";
    }
    else if (accion == 'Q'){
      cout << "Se ha solicitado el cierre del chat!.\n";
    }
  }while(accion != 'Q');

  shutdown(socket_cliente, SHUT_RDWR);
  close(socket_cliente);

  cout << "Read_thread termino.\n";
}
void write_thread(int socket_cliente) {
  char buffer[100];
  string  txt;
  char accion;
  bool  nicknameOn = false;
  int n;
  do{
    if (! nicknameOn){
      string a="N";
      cout << "Ingrese su Nickname:";
      cin >>  txt;
      nicknameOn = true;
      //buffer[0]='N';
      sprintf(buffer,"%03d",txt.size());
      buffer[4]='\0';
      string t = buffer;
      string tmp = a + t + txt;
      strcpy(buffer,tmp.c_str());
      cout << "Protocolo:" << tmp << endl;
      n = write(socket_cliente,buffer,tmp.size());
    }
    cout << "MSG: ";
    cin >> txt;
    if (txt.compare("Q") == 0 ){
      buffer[0]='Q';
      buffer[1]='0';
      buffer[2]='0';
      buffer[3]='0';
      n = write(socket_cliente,buffer,4);
      accion = 'Q';
    }
    else{ // msg
      string a="M";
      sprintf(buffer,"%03d",txt.size());
      buffer[4]='\0';
      string t = buffer;
      string tmp = a +  t + txt;
      strcpy(buffer,tmp.c_str());
      cout << "Protocolo:" << tmp << endl;
      n=write(socket_cliente,buffer,tmp.size());
    }    
  }while(accion!='Q');
  shutdown(socket_cliente, SHUT_RDWR);
  close(socket_cliente);

}


  int main(void)
  {
    struct sockaddr_in stSockAddr;
    int Res;
    int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    int n;
    char buffer[1000];
 
    if (-1 == SocketFD)
    {
      perror("cannot create socket");
      exit(EXIT_FAILURE);
    }
 
    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));
 
    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(8080);
    Res = inet_pton(AF_INET, "127.0.0.1", &stSockAddr.sin_addr);
 
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

  std::thread (read_thread,SocketFD).detach();
  std::thread (write_thread,SocketFD).detach();

//      std::thread (READ,SocketFD).detach();
//      std::thread (WRITE,SocketFD).detach();

  while(true){};

   // n = write(SocketFD,"Hi, this is Bob.",18);
    /* perform read write operations ... */
 

    return 0;
  }
