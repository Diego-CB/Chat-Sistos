#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

// Definir las estructuras del protocolo
#include "./Protocol/chat.pb-c.h"

int main(int argc, char *argv[]) {

  // Variables pasadas como parametro de consola
  char* user_name = argv[1];
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

  // Serializar mensaje UserOption
  size_t user_option_size = chat_sist_os__user_option__get_packed_size(&user_option);
  uint8_t *user_option_buf = malloc(user_option_size);
  chat_sist_os__user_option__pack(&user_option, user_option_buf);

  // Enviar mensaje UserOption al servidor
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
  
  return 0;
}
  /*
  // Menu para recibir acciones del usuario
  
    1. Chatear con todos los usuarios (broadcasting).
    2. Enviar y recibir mensajes directos, privados, aparte del chat general.
    3. Cambiar de status.
    4. Listar los usuarios conectados al sistema de chat.
    5. Desplegar información de un usuario en particular.
    6. Ayuda.
    7. Salir.
  int opcion_menu = 0;
  while (opcion_menu != 7)
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
    if (opcion_menu == 1)
    {
      string mensaje_para_enviar;
      printf("Ingrese el mensaje a continuacion: ");
      scanf("%d", &mensaje_para_enviar);
      printf("\n");
      
      // Crear useroption
      UserOption userOption = USER_OPTION__INIT;
      userOption.op = 5; // Opción de enviar un mensaje

      // Crear mensaje
      Message message = MESSAGE__INIT;
      message.message_private = False; // Mensaje privado
      message.message_destination = ""; // Nombre de usuario destino
      message.message_content = mensaje_para_enviar; // Contenido del mensaje
      message.message_sender = "UsuarioOrigen"; // Nombre de usuario que envía el mensaje

      // agregar mensaje a useroption
      userOption.message = &message;
    }
    else if (opcion_menu == 2)
    {
      printf("2\n");
    }
    else if (opcion_menu == 3)
    {
      printf("3\n");
      
    }
    else if (opcion_menu == 4)
    {
      printf("4\n");
      
    }
    else if (opcion_menu == 5)
    {
      printf("5\n");
      
    }
    else if (opcion_menu == 6)
    {
      printf("6\n");
      
    }
    else if (opcion_menu == 7)
    {
      printf("Saliendo del chat.\n");
    }
}
    

    
    // Crear una estructura de UserOption para enviar al servidor
    UserOption userOption = USER_OPTION__INIT;
    userOption.op = 5; // Opción de enviar un mensaje
    Message message = MESSAGE__INIT;
    message.message_private = 1; // Mensaje privado
    message.message_destination = "UsuarioDestino"; // Nombre de usuario destino
    message.message_content = "Hola, ¿cómo estás?"; // Contenido del mensaje
    message.message_sender = "UsuarioOrigen"; // Nombre de usuario que envía el mensaje
    userOption.message = &message;

    // Serializar la estructura de UserOption para enviarla al servidor
    uint8_t buffer[1024];
    unsigned int length = user_option__get_packed_size(&userOption);
    user_option__pack(&userOption, buffer);

    // Enviar el mensaje al servidor
    if (send(sock, buffer, length, 0) < 0) {
        printf("Error al enviar el mensaje");
        return 1;
    }

    // Recibir la respuesta del servidor
    uint8_t response_buffer[1024];
    int response_length = recv(sock, response_buffer, 1024, 0);
    if (response_length < 0) {
        printf("Error al recibir la respuesta del servidor");
        return 1;
    }

    // Deserializar la respuesta del servidor
    Answer *answer = answer__unpack(NULL, response_length, response_buffer);
    if (answer == NULL) {
        printf("Error al deserializar la respuesta del servidor");
        return 1;
    }

    // Procesar la respuesta del servidor
    if (answer->response_status_code == 200) {
        printf("Mensaje enviado correctamente\n");
    } else {
        printf("Error al enviar el mensaje: %s\n", answer->response_message);
    }

    // Liberar la memoria utilizada
    answer__free_unpacked(answer, NULL);
    close(sock);

    return 0;
  */
