#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <chat.pb-c.h>

int main(int arg, char **args)
{
  struct ChatSistOS__UserOption cadena;       // lo que recibe el servidor de los clientes.
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
  if (
      bind(
        listen_socket,
        (struct sockaddr *)&servidorADDR,
        sizeof(servidorADDR)
      ) < 0
    ) {
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
    /*
    ChatSistOS__UserList
    ChatSistOS__UsersOnline
    ChatSistOS__UserOption
    ChatSistOS__Answer
    ChatSistOS__User
    ChatSistOS__NewUser
    ChatSistOS__Status
    ChatSistOS__Message
    */
    
    tiempo = time(NULL);
    tm = localtime(&tiempo);
    strftime(horaMensaje, 100, "(%H:%M)", tm); // hora que se recibe el mensaje

    struct ChatSistOS__Answer respuesta;
    respuesta.op = cadena.op;
    respuesta.response_status_code = 200;
    
    // opciones de acción del servidor
    /*
      1. Chatear con todos los usuarios (broadcasting).
      2. Enviar y recibir mensajes directos, privados, aparte del chat general.
      3. Cambiar de status.
      4. Listar los usuarios conectados al sistema de chat.
      5. Desplegar información de un usuario en particular.
    */
    if (cadena.op == 0)
    {
      
      struct ChatSistOS__NewUser new_user = cadena.createuser;
      char* new_username = new_user.username;
      char* new_ip = new_user.ip;
      // TODO Revisar si el usuario ya existe o no
      // TODO registrar nuevo usuario
    }
    else if (cadena.op == 1 || cadena.op == 2) // Enviar Mensaje
    {
      struct ChatSistOS__Message new_message = cadena.message;
      char* content = new_message.message_content;
      char* sender = new_message.message_sender;

      /**
        TODO
        respuesta.response_message = ?
        respuesta.message = ?
      */
      
      // Mensajes privados
      if (new_message.message_private)
      {
        char* destination = new_message.message_destination;
        // TODO Mandar mensaje privado
      }
      else
      {
        // TODO Mandar mensaje publico
      }
    }
    else if (cadena.op == 3) // Cambiar estatus
    {
      /**
        TODO
        respuesta.status = ?
      */
      
      struct ChatSistOS__Status new_status = cadena.status;
      char* username = new_status.user_name;
      int new_state = new_status.user_state;

      // TODO cambiar estado
    }
    else if (cadena.op == 4 || cadena.op == 5) // Listar los usuarios conectados
    {
      struct ChatSistOS__Status userlist = cadena.userlist;

      // Listado de usuarios
      if (userlist.list)
      {
        /**
          TODO
          respuesta.users_online = ?
        */
        // TODO mandar listado de usuarios  
      }
      else // Usuario especifico
      {
        /**
          TODO
          respuesta.user = ?
        */
        char* user_to_see = userlist.user_name;
        // TODO mandar info del usuario
      }
    }
    else
    {
      respuesta.response_status_code = 400;
      //TODO mandar mensaje de error
    }
    
  }

  return 0;
}

/*
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
*/