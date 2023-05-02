#include "./Protocol/chat.pb-c.h"
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>

int main(int argc, char **argv) {
  // pthreads
  pthread_t pthread_id;

  // Ruta para coneccion
  int puerto = atoi(argv[1]);
  char *direccion_ip = "127.0.0.1";

  // Configurar el Socket del servidor
  ChatSistOS__UserOption *cadena; // lo que recibe el servidor de los clientes.
  int listen_socket;              // para crear socket
  int clientes_en_espera; // parar recoger a los clientes en lista de espera.
  struct sockaddr_in servidorADDR; // para crear socket

  time_t tiempo; // hora actual
  struct tm *tm;
  char horaMensaje[100];
  char *tmp;

  // char mensaje_a_cliente[100] = ""

  // Crear Socket
  listen_socket = socket(AF_INET, SOCK_STREAM, 0);

  if (listen_socket == -1) {
    // fallo crear socket
    return -1;
  }

  bzero(&servidorADDR, sizeof(servidorADDR));
  servidorADDR.sin_family = AF_INET;
  servidorADDR.sin_addr.s_addr = inet_addr(direccion_ip);
  servidorADDR.sin_port = htons(puerto);
  

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
  struct sockaddr_in clienteADDR; // para conectar sockets

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

    ChatSistOS__Answer *respuesta;
    respuesta->op = cadena->op;
    respuesta->response_status_code = 200;

    // opciones de acción del servidor
    /*
      1. Chatear con todos los usuarios (broadcasting).
      2. Enviar y recibir mensajes directos, privados, aparte del chat general.
      3. Cambiar de status.
      4. Listar los usuarios conectados al sistema de chat.
      5. Desplegar información de un usuario en particular.
    */
    if (cadena->op == 0) {

      ChatSistOS__NewUser *new_user = cadena->createuser;
      char *new_username = new_user->username;
      char *new_ip = new_user->ip;
      // TODO Revisar si el usuario ya existe o no
      // TODO registrar nuevo usuario
    } else if (cadena->op == 1 || cadena->op == 2) // Enviar Mensaje
    {
      ChatSistOS__Message *new_message = cadena->message;
      char *content = new_message->message_content;
      char *sender = new_message->message_sender;

      /**
        TODO
        respuesta.response_message = ?
        respuesta.message = ?
      */

      // Mensajes privados
      if (new_message->message_private) {
        char *destination = new_message->message_destination;
        // TODO Mandar mensaje privado
      } else {
        // TODO Mandar mensaje publico
      }
    } else if (cadena->op == 3) // Cambiar estatus
    {
      /**
        TODO
        respuesta.status = ?
      */

      ChatSistOS__Status *new_status = cadena->status;
      char *username = new_status->user_name;
      int new_state = new_status->user_state;

      // TODO cambiar estado
    } else if (cadena->op == 4 ||
               cadena->op == 5) { // Listar los usuarios conectados
      ChatSistOS__UserList *server_userlist = cadena->userlist;
      // Listado de usuarios
      if (server_userlist->list) {
        /**
          TODO
          respuesta.users_online = ?
        */
        // TODO mandar listado de usuarios
      } else // Usuario especifico
      {
        /**
          TODO
          respuesta.user = ?
        */
        char *user_to_see = userlist->user_name;
        // TODO mandar info del usuario
      }
    } else {
      respuesta->response_status_code = 400;
      // TODO mandar mensaje de error
    }
  }

  return 0;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage: %s <port>\n", argv[0]);
    return EXIT_FAILURE;
  }

  char *ip = "127.0.0.1";
  int port = atoi(argv[1]);

  int option = 1;

  int listenfd = 0, connfd = 0;
  struct sockaddr_in serv_addr;
  struct sockaddr_in cli_addr;
  pthread_t tid;

  // Socket settings
  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = inet_addr(ip);
  serv_addr.sin_port = htons(port);

  // Ignore pipe signals
  signal(SIGPIPE, SIG_IGN);

  if (setsockopt(listenfd, SOL_SOCKET, (SO_REUSEPORT | SO_REUSEADDR),
                 (char *)&option, sizeof(option)) < 0) {
    perror("ERROR: setsockopt failed");
    return EXIT_FAILURE;
  }

  // Bind
  if (bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    perror("ERROR: Socket binding failed");
    return EXIT_FAILURE;
  }

  // Listen
  if (listen(listenfd, 10) < 0) {
    perror("ERROR: Socket listening failed");
    return EXIT_FAILURE;
  }

  printf("=============================\n");
  printf("\tCHATROOM el buen samaritano\n");
  printf("=============================\n");

  while (1) {
    socklen_t clilen = sizeof(cli_addr);
    connfd = accept(listenfd, (struct sockaddr *)&cli_addr, &clilen);

    // Check for max clients
    if ((cli_count + 1) == MAX_CLIENTS) {
      printf("Max clients reached. Rejected: ");
      print_client_addr(cli_addr);
      printf(":%d\n", cli_addr.sin_port);
      close(connfd);
      continue;
    }

    // Client settings
    client_t *cli = (client_t *)malloc(sizeof(client_t));
    cli->address = cli_addr;
    cli->sockfd = connfd;
    cli->uid = uid++;
    cli->status = 1;

    // Add client to the queue and fork thread
    queue_add(cli);
    pthread_create(&tid, NULL, &handle_client, (void *)cli);

    // Reduce CPU usage
    sleep(1);
  }

  return EXIT_SUCCESS;
}
