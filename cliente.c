#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>


// Definir las estructuras del protocolo
#include "./Protocol/chat.pb-c.h"

// Prototipos de funciones a usar
void print_ayuda();
int get_user_input();
void ver_mensajes_recibidos(int);
ChatSistOS__Answer* send_user_option(ChatSistOS__UserOption, int);

// Main
int main(int argc, char *argv[]) {

  // Variables pasadas como parametro de consola
  char user_name[101];    
  strcpy(user_name, argv[1]);
  char* ip_server = argv[2];
  int puerto_server = atoi(argv[3]);
  
  // Crear un socket
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == -1) {
    printf("Error al crear el socket");
    return 1;
  }

  // Especificar la dirección IP y el puerto del servidor
  struct sockaddr_in server;
  server.sin_addr.s_addr = inet_addr(ip_server); // Dirección IP del servidor
  server.sin_family = AF_INET;
  server.sin_port = htons(puerto_server); // Puerto del servidor

  // Conectar al servidor
  if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
    printf("Error al conectarse al servidor. Puede que la demanda sea alta.");
    return 1;
  }

  struct sockaddr_in client;
  socklen_t client_len = sizeof(client);
  getpeername(sock, (struct sockaddr *)&client, &client_len);
  char *client_ip = inet_ntoa(client.sin_addr);
  char* client_ip_str = strdup(client_ip);

  // Crear mensaje UserOption para crear un nuevo usuario
  ChatSistOS__UserOption user_option = CHAT_SIST_OS__USER_OPTION__INIT;
  user_option.op = 2; // Crear nuevo usuario
  ChatSistOS__NewUser new_user = CHAT_SIST_OS__NEW_USER__INIT;
  new_user.username = user_name;
  new_user.ip = client_ip_str;
  user_option.createuser = &new_user;

  // Enviar mensaje al server y recibir respuesta
  ChatSistOS__Answer* answer = send_user_option(user_option, sock);

  if (answer == NULL) {
    fprintf(stderr, "Error al deserializar mensaje Answer\n");
    exit(EXIT_FAILURE);
  }

  // Verificar si el usuario fue creado exitosamente
  if (answer->response_status_code == 200) {
    printf("Usuario creado exitosamente\n");
  } else {
    printf("Error al crear usuario: %s\n", answer->response_message);
  }

  // Menu principal
  int opcion_menu = 0;
  while (opcion_menu != 7) 
  {
    // Print de mensajes recibidos
    ver_mensajes_recibidos(sock);

    // Obtener input del usuario
    opcion_menu = get_user_input();

    // Crear USER_OPTION para mandar al sever
    ChatSistOS__UserOption user_option_menu = CHAT_SIST_OS__USER_OPTION__INIT;

    if (opcion_menu == 1) // Broadcast
    {
      user_option_menu.op = 1;
      int valor_opcion_chat_general = 0;
      while (valor_opcion_chat_general != 2) {

        printf(" 1 -> Enviar mensaje nuevo al chat general\n");
        printf(" 2 -> Regresar al menu principal\n");
        printf(" Ingresa la opción que deseas ejecutar (1|2):  ");
        scanf("%d", &valor_opcion_chat_general);
        printf("\n");

        if (valor_opcion_chat_general < 1 || valor_opcion_chat_general > 2)
        {
          printf("\nERROR: Ingrese una opción valida\n\n");
        }
        else if (valor_opcion_chat_general == 1)
        {
          char nuevoMensaje[1024];
          printf(" Ingresa el mensaje:  ");
          scanf("%s", nuevoMensaje);
          printf("\n");
          ChatSistOS__Message nuevo_mensaje_general = CHAT_SIST_OS__MESSAGE__INIT;
          nuevo_mensaje_general.message_private = 0;
          nuevo_mensaje_general.message_sender = user_name; 
          nuevo_mensaje_general.message_content = nuevoMensaje;

          user_option_menu.message = &nuevo_mensaje_general;

          //mandar el user option al servidor
          ChatSistOS__Answer* respuesta = send_user_option(user_option_menu, sock);

          if (respuesta -> response_status_code == 200) {
            printf(" * enviado *\n");
          } else {
            printf("Error No se ha enviado el mensaje\n");
          } 
        }
      }
      
    }
    else if (opcion_menu == 2) // Inbox
    {
      user_option_menu.op = 2;
      int valor_opcion_chat_general = 0;
      while (valor_opcion_chat_general != 2) {

        char cliente_para_mensaje_priv[101];
        printf(" A quien deseas enviar el mensaje:  ");
        scanf("%s", cliente_para_mensaje_priv);
        printf("\n");

        printf(" 1 -> Enviar otro mensaje a %s\n", cliente_para_mensaje_priv);
        printf(" 2 -> Regresar al menu principal\n");
        printf(" Ingresa la opción que deseas ejecutar (1|2):  ");
        scanf("%d", &valor_opcion_chat_general);
        printf("\n");

        if (valor_opcion_chat_general < 1 || valor_opcion_chat_general > 2)
        {
          printf("\nERROR: Ingrese una opción valida\n\n");
        }
        else if (valor_opcion_chat_general == 1)
        {
          char nuevoMensaje[1024];
          printf(" Ingresa el mensaje:  ");
          scanf("%s", nuevoMensaje);
          printf("\n");
          ChatSistOS__Message nuevo_mensaje_priv = CHAT_SIST_OS__MESSAGE__INIT;
          nuevo_mensaje_priv.message_private = 0;
          nuevo_mensaje_priv.message_sender = user_name; 
          nuevo_mensaje_priv.message_content = nuevoMensaje;
          nuevo_mensaje_priv.message_destination = cliente_para_mensaje_priv;

          user_option_menu.message = &nuevo_mensaje_priv;

          //mandar el user option al servidor
          ChatSistOS__Answer* respuesta = send_user_option(user_option_menu, sock);

          if (respuesta -> response_status_code == 200) {
            printf(" * enviado *\n");
          } else {
            printf("Error No se ha enviado el mensaje\n");
          } 
        }
      }
    }
    else if (opcion_menu == 3) // Cambiar estatus
    {
      // obtener nuevo status
      int valor_nuevo_status = 0;

      while (valor_nuevo_status < 1 || valor_nuevo_status > 3)
      {
        printf(" 1 -> Online\n");
        printf(" 2 -> Busy\n");
        printf(" 3 -> Disconnected\n");
        printf("Ingresa tu nuevo status (1|2|3):  ");
        scanf("%d", &valor_nuevo_status);
        printf("\n");

        if (opcion_menu < 1 || opcion_menu > 3)
        {
          printf("\nERROR: Ingrese una opcon valida\n\n");
        }
      }

      // modificamos la user option que se enviara al servidor
      user_option_menu.op = 3;
      ChatSistOS__Status nuevo_status = CHAT_SIST_OS__STATUS__INIT;
      nuevo_status.user_name = user_name;
      nuevo_status.user_state = valor_nuevo_status;
      user_option_menu.status = &nuevo_status;

      // Mandar user Option
      ChatSistOS__Answer* respuesta = send_user_option(user_option_menu, sock);
      
      // Verificar si el usuario fue creado exitosamente
      if (respuesta -> response_status_code == 200) {
        printf("Estatus cambiado Existosamente\n");
      } else {
        printf("Error al cambiar estatus: %s\n", answer -> response_message);
      }
    }
    else if(opcion_menu == 4) // Lista de usuarios conectados
    {
      // Llenar UserOption
      user_option_menu.op = 4;
      ChatSistOS__UserList listar_usuarios_userlist_todos = CHAT_SIST_OS__USER_LIST__INIT;
      listar_usuarios_userlist_todos.list = 0;

      // Agregar el userlist a la user option.
      user_option_menu.userlist = &listar_usuarios_userlist_todos;

      // Mandar user Option
      ChatSistOS__Answer* respuesta = send_user_option(user_option_menu, sock);

      // Verificar respueta del server
      if (respuesta -> response_status_code != 200) {
        printf("Error al obtener lista dde usuarios conectados\n");
        continue;
      } else {
        // Desempquetar buffer
        uint8_t buffer_users_online[4096];
        printf("antes unpack\n");
        ChatSistOS__UsersOnline *decoded_users_online = chat_sist_os__users_online__unpack(
          NULL,
          (int) sizeof(buffer_users_online),
          buffer_users_online
        );

        // Print del listado
        printf("Numero de usuarios en lineaL %d\n", (int) decoded_users_online -> n_users);
        printf("---- Usuarios en Linea ----\n");
        for (int i = 0; i < decoded_users_online -> n_users; i++)
        {
          ChatSistOS__User *user = decoded_users_online -> users[i];
          printf("%d: %s (ip: %s)\n", i+1, user -> user_name, user -> user_ip);
        }
      }
    }
    else if(opcion_menu == 5) // Info de usuario particular
    {
      user_option_menu.op = 5;
      ChatSistOS__UserList listar_usuarios_userlist = CHAT_SIST_OS__USER_LIST__INIT;

      char usuario_especifico_listar[101];
      printf("Ingresa el nombre del usuario: ");
      scanf("%s", usuario_especifico_listar);
      printf("\n");

      listar_usuarios_userlist.list = 0;
      listar_usuarios_userlist.user_name = usuario_especifico_listar;

      // Agregar el userlist a la user option.
      user_option_menu.userlist = &listar_usuarios_userlist;

      // Mandar user Option
      ChatSistOS__Answer* respuesta = send_user_option(user_option_menu, sock);

      // Verificar respueta del server
      if (respuesta -> response_status_code != 200) {
        printf("Error al obtener lista dde usuarios conectados\n");
        continue;
      }

      // Desempaquetar usuario
      ChatSistOS__User *user_option_5 = respuesta -> user;

      // Print de info del usuario
      printf("%s (ip: %s)\n", user_option_5 -> user_name, user_option_5 -> user_ip);
    }
    else if(opcion_menu == 6) // Imprimir ayuda
    {
      print_ayuda();
    }
    else if(opcion_menu == 7) // Salir
    {
      close(sock);
      printf("Gracias por utilizar el chat!\n");
    }
  }
  
  return 0;
}

void ver_mensajes_recibidos(int sock)
{
  // Recibir mensajes
  uint8_t buffer_mensaje[4096];
  ssize_t bytes_mensaje = recv(sock, buffer_mensaje, sizeof(buffer_mensaje), 0);
  if (bytes_mensaje < 0) {
    printf("---- Mensajes Recibido ----\n");

    // Deserializar mensaje Answer
    ChatSistOS__Answer *answer_mensaje_recibido = chat_sist_os__answer__unpack(NULL, bytes_mensaje, buffer_mensaje);
    if (answer_mensaje_recibido ->response_status_code == 200)
    {
      ChatSistOS__Message *mensaje_recibido = answer_mensaje_recibido -> message;
      char* mensaje_text = mensaje_recibido -> message_content;

      // Mensaje privado
      if (mensaje_recibido -> message_private)
      {
        char* sender  = mensaje_recibido -> message_sender;
        printf("> [%s]: %s\n", sender, mensaje_recibido);
      }
      else
      {
        printf("> [Broadcast]: %s\n", mensaje_recibido);
      }
    }
  }
}

/**
 * Envia un UserOption al server
 * Obtiene la respuesta del servidor
 * Devuelve la respuesta del servidor
*/
ChatSistOS__Answer* send_user_option(ChatSistOS__UserOption user_option, int sock)
{
  // Serializar mensaje UserOption
  size_t user_option_size = chat_sist_os__user_option__get_packed_size(&user_option);
  uint8_t *user_option_buf = malloc(user_option_size);
  chat_sist_os__user_option__pack(&user_option, user_option_buf);

  // Enviar mensaje
  if (send(sock, user_option_buf, user_option_size, 0) == -1) {
    perror("send");
    exit(EXIT_FAILURE);
  }

  // Recibir respuesta del servidor
  uint8_t buf[4096];
  ssize_t bytes_received = recv(sock, buf, sizeof(buf), 0);
  if (bytes_received == -1) {
    perror("recv");
    exit(EXIT_FAILURE);
  }

  // Deserializar mensaje Answer
  ChatSistOS__Answer *answer = chat_sist_os__answer__unpack(NULL, bytes_received, buf);
  return answer;
}

/**
 * Obtiene input del usuario
*/
int get_user_input()
{
  int opcion_menu = 0;

  while (opcion_menu < 1 || opcion_menu > 7)
  {
    printf("\n");
    printf("1. Chatear con todos los usuarios (broadcasting).\n");
    printf("2. Enviar y recibir mensajes directos, privados, aparte del chat general.\n");
    printf("3. Cambiar de status.\n");
    printf("4. Listar los usuarios conectados al sistema de chat.\n");
    printf("5. Desplegar información de un usuario en particular.\n");
    printf("6. Ayuda.\n");
    printf("7. Salir.\n");
    printf("-> ");
    scanf("%d", &opcion_menu);

    if (opcion_menu < 1 || opcion_menu > 7)
    {
      printf("\nERROR: Ingrese una opcon valida\n\n");
    }
  }
  return opcion_menu;
}

/**
 * Imprime las aydas del menu
*/
void print_ayuda()
{
  printf("\033[1mBienvenido a la sección de ayuda\033[0m\n\n");
  printf("En el chat encontraras diversas instrucciones y se te solicitará ingresar un comando. Los comandos en el menú actualmente son:\n");
  printf("\t->  1 (\033[1mChatear con todos los usuario\033[0m):\n");
  printf("\t\tPodrás enviar y recibir mensajes todos los usuarios dentro del servidor. Se te solicitará escribir el mensaje que deseas enviar y se mostrarán los mensajes que se han enviado si ese es el caso.\n");
  printf("\t->  2 (\033[1mEnviar y recibir mensajes directos\033[0m):\n");
  printf("\t\tPodrás enviar y recibir mensajes directos con otros usuarios dentro del servidor. Se te solicitará escribir el mensaje que deseas enviar y se mostrará el mensaje que te han enviado si ese es el caso.\n");
  printf("\t->  3 (\033[1mCambiar de status\033[0m):\n");
  printf("\t\tCambia el status de tu usuario en el servidor. Puedes mostrarte como disponible, ocupado y ausente. Se te mostrará en pantalla el numero correspondiente para poder asignarte este status y deberás de ingresar el mismo para que todos los usuarios conectados al servidor puedan ver dicho status.\n");
  printf("\t->  4 (\033[1mListar los usuarios conectados al sistema de chat\033[0m):\n");
  printf("\t\tSe te mostrará un listado de todos los usuarios conectados actualmente en el chat.\n");
  printf("\t->  5 (\033[1mDesplegar información de un usuario en específico\033[0m):\n");
  printf("\t\tSe te solicitará ingresar el nombre de usuario de algún usuario y si dicho usuario está conectado se te mostrará la información de su usuario (nombre y status).\n");
  printf("\t->  6 (\033[1mAyuda\033[0m):\n");
  printf("\t\tDesplegar sección de ayuda del programa.\n");
  printf("\t->  7 (\033[1msalir\033[0m):\n");
  printf("\t\tEjecuta este comando para salir del programa y terminar tu conexión con el servidor.\n\n");
  printf("Se te solicitará llenar los campos necesarios a medida que vas interactuando en el chat. Si tienes problemas de conexión en el futuro se puede deber a que la demanda del servidor es alta. Recuerda que al ejecutar tu cliente en consola debes de ingresar:\n");
  printf("\t->  Nombre de usuario\n");
  printf("\t->  IP del servidor\n");
  printf("\t->  Puerto del servidor\n\n");
  printf("\033[1mEsperamos disfrutes tu experiencia! Gracias por utilizar nuestro chat.\033[0m\n");
}
