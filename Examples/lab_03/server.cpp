  /* Server code in C */
 
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
 #include <map>

using namespace  std;


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
      cout << "New Cliente:" << buffer << ":";
      room[socket_cliente] = buffer;
      broadcast(buffer);
    }
    else  if (accion == 'M'){
      n = read(socket_cliente,buffer,tamano);
      buffer[n]='\0';
      cout << buffer << "\n";
      broadcast(buffer);
    }
    else if (accion == 'Q'){
      cout << "Se ha solicitado el cierre del chat!.\n";
      string nick = room[socket_cliente];
      room.erase (socket_cliente);
      n = write(socket_cliente,"Q000",4);
      broadcast("user se fue! " + nick); 
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
    int SocketServer = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    char buffer[256];
    int n;
 
    if(-1 == SocketServer)
    {
      perror("can not create socket");
      exit(EXIT_FAILURE);
    }
 
    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));
 
    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(8080);
    stSockAddr.sin_addr.s_addr = INADDR_ANY;
 
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
 
    for(;;) // muchos  clientes      
    {
      int newSocketCli = accept(SocketServer, NULL, NULL);
      if(0 > newSocketCli)
      {
        perror("error accept failed");
        close(SocketServer);
        exit(EXIT_FAILURE);
      }
 
   std::thread (read_thread,newSocketCli).detach();
   std::thread (write_thread,newSocketCli).detach();
//   std::thread (READ,newSocketCli).detach();
//   std::thread (WRITE,newSocketCli).detach();

    }
 
    close(SocketServer);
    return 0;
  }
