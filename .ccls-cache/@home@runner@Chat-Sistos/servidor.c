#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

struct UserOption {
  // opcion a realizar
  int op;

  Message message;
}

struct Message {

}


int main(int arg, char **args) {
  char cadena[100];       // lo que recibe el servidor de los clientes.
  int listen_socket;      // para crear socket
  int clientes_en_espera; // parar recoger a los clientes en lista de espera.
  struct sockaddr_in servidorADDR; // para crear socket

  time_t tiempo; // hora actual
  struct tm *tm;
  char horaMensaje[100];
  char *tmp;

  // char mensaje_a_cliente[100] = ""

  // -------------------------------------- Socket
  // ----------------------------------------------
  listen_socket = socket(AF_INET, SOCK_STREAM, 0);

  if (listen_socket == -1) {
    // fallo crear socket
    return -1;
  }

  bzero(&servidorADDR, sizeof(servidorADDR));
  servidorADDR.sin_family = AF_INET;
  servidorADDR.sin_addr.s_addr = INADDR_ANY;
  servidorADDR.sin_port = *args[0];

  // bind del cliente con el servidor
  if (bind(listen_socket, (struct sockaddr *)&servidorADDR,
           sizeof(servidorADDR)) < 0) {
    return -1;
  }

  // escuchar del servidor para recibir clientes.
  if (listen(listen_socket, 10) < 0) {
    return -1;
  }

  // recoger los clientes de la lista de espera
  clientes_en_espera = accept(listen_socket, (struct sockaddr *)NULL, NULL);

  while (1) {
    bzero(cadena, 100); // limpiar cadena

    if (read(clientes_en_espera, cadena, 100) < 0) {
      return -1;
    }
    tiempo = time(NULL);
    tm = localtime(&tiempo);
    strftime(horaMensaje, 100, "(%H:%M)", tm); // hora que se recibe el mensaje
  }

  return 0;
}

#include <arpa/inet.h>
#include <ctype.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  int sockfd = 0;
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (sockfd == -1) {
    printf("Fail to create a socket.");
  }

  struct sockaddr_in info;
  bzero(&info, sizeof(info));
  info.sin_family = PF_INET;

  info.sin_addr.s_addr = inet_addr("127.0.0.1");
  info.sin_port = htons(8700);

  int err = connect(sockfd, (struct sockaddr *)&info, sizeof(info));
  if (err == -1) {
    printf("Connection error");
  }

  char receiveMessage[100] = {};
  char message[] = {"Hi there"};
  send(sockfd, message, sizeof(message), 0);
  recv(sockfd, receiveMessage, sizeof(receiveMessage), 0);

  printf("%s", receiveMessage);
  printf("close Socket\n");
  close(sockfd);
  return 0;
}
