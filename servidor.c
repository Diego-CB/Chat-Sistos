#include "./Protocol/chat.pb-c.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/ip.h>
#include <sys/types.h>
#include <time.h>

#define LIMITE_SESIONES 10
// Definir lista de mensajes broadcast para ser accesada por manejar comunicaciones.

pthread_mutex_t structure_mutex = PTHREAD_MUTEX_INITIALIZER;
static int conteo_clientes = 0;

// Estructura para crear clientes
typedef struct {
  struct sockaddr_in clienteADDR;
  long sockfd;
  int stats;
  char name[101];
  char* direccion_ip;
} info_cliente_;

// Lista de clientes conectados al servidor
info_cliente_ clientes[LIMITE_SESIONES];

// Numero de clientes conectados al servidor
int num_clientes = 0;

// Prototipo de subrutina para threads
void* manejar_comunicaciones(void* arg);

// Main
int main(int argc, char **argv)
{
  // Ruta para coneccion
  int puerto = atoi(argv[1]);
  char *ip_server = "18.221.21.104";

  // Crear un socket para escuchar conexiones entrantes
  int server_sock = socket(AF_INET, SOCK_STREAM, 0);
  if (server_sock == -1) {
      printf(" > Error al crear el socket");
      return 1;
  }

  // Especificar la direccion IP y el puerto del servidor
  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(puerto);

  // Vincular el socket con la direccion IP y el puerto
  if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    printf(" > Error al vincular el socket");
    return 1;
  }

  // Esperar conexiones entrantes
  if (listen(server_sock, 1) < 0) {
    printf(" > Error al esperar conexiones entrantes");
    return 1;
  }

  // Iniciar mutex
  pthread_mutex_init(&structure_mutex, NULL);
  printf("\nSERVER RUNNING AT PORT %d!\n", puerto);
  // Aceptar conexiones entrantes
  while (1) {
    // Aceptar la conexion entrante
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_len);
    if (client_sock < 0) {
      printf(" > Error al aceptar la conexion entrante");
      continue;
    }

    // AQUI RECIBE EL USER OPTION PARA CREAR USUARIO
    uint8_t buf[4096];
    ssize_t bytes_recieved = recv(client_sock, buf, sizeof(buf), 0); // VERIFICAR PQ AQUI VA EL SOCKET DE USUARIO
    if (bytes_recieved == -1) {
        perror("> Error en recv");
        exit(EXIT_FAILURE);
    }

    ChatSistOS__UserOption *user_option_createUser = chat_sist_os__user_option__unpack(NULL, bytes_recieved, buf);
    user_option_createUser -> op = 2; // Crear nuevo usuario

    // New User Structure
    ChatSistOS__NewUser *nuevo_usuario = user_option_createUser -> createuser;

    char* ip_usuario = nuevo_usuario -> ip;
    printf("%s\n", ip_usuario);
    
    char nombre_usuario[101];    
    strcpy(nombre_usuario, nuevo_usuario -> username);


    // Check if username already exists in the array
    int username_exists = 0;
    for (int i = 0; i < num_clientes; i++) {
      if (strcmp(clientes[i].name, nombre_usuario) == 0) {
        username_exists = 1;
        break;
      }
    }

    // Verificar si hay menos de MAX_CLIENTES clientes conectados actualmente
    if (num_clientes < LIMITE_SESIONES && !username_exists) {
      // Agregar el nuevo cliente a la lista de clientes conectados
      clientes[num_clientes].sockfd = client_sock;
      clientes[num_clientes].clienteADDR = client_addr;
      clientes[num_clientes].stats = 1;      
      strcpy(clientes[num_clientes].name, nombre_usuario); //cambiar por el del usuario
      clientes[num_clientes].direccion_ip = ip_usuario; //cambiar por el del usuario

      num_clientes++; // suma al contador de clientes

      // ARMAR ANSWER PARA ENVIAR Y DECIR SI SE LOGRO CREAR USER
      ChatSistOS__Answer respuesta_success_userCreate = CHAT_SIST_OS__ANSWER__INIT;
      respuesta_success_userCreate.op = 2; // Opcion de crear usuario VERIFICAR
      respuesta_success_userCreate.response_status_code = 200;
      respuesta_success_userCreate.response_message = " > Usuario creado con exito!";

      //ENVIARLE AL USUARIO LA RESPUESTA
      //serializar
      size_t respuesta_success_userCreate_size = chat_sist_os__answer__get_packed_size(&respuesta_success_userCreate);
      uint8_t *respuesta_success_userCreate_buf = malloc(respuesta_success_userCreate_size);
      chat_sist_os__answer__pack(&respuesta_success_userCreate, respuesta_success_userCreate_buf);

      //enviar - FALTA MODIFICAR SOCK POR VALORES EN SERVIDOR
      if (send(client_sock, respuesta_success_userCreate_buf, respuesta_success_userCreate_size, 0) == -1) {
        perror(" > Error en send");
        exit(EXIT_FAILURE);
      }

      // IMLEMENTACION DE THREADS
      // Crear un hilo para manejar conexiones con ese cliente
      pthread_t hilo;
      if (pthread_create(&hilo, NULL, manejar_comunicaciones, (void*)&clientes[num_clientes-1]) < 0) {
        printf(" > Error al crear un nuevo hilo para manejar las comunicaciones con el cliente\n");
        continue;
      }

    } else {

      // ARMAR ANSWER PARA ENVIAR Y DECIR NO SE LOGRO CREAR USER
      ChatSistOS__Answer respuesta_failed_userCreate = CHAT_SIST_OS__ANSWER__INIT;
      respuesta_failed_userCreate.op = 2; // Opcion de crear usuario VERIFICAR
      respuesta_failed_userCreate.response_status_code = 400;
      respuesta_failed_userCreate.response_message = " > Error: Usuario no se ha podido crear por alta demanda.\n";

      //ENVIARLE AL USUARIO LA RESPUESTA
      //serializar
      size_t respuesta_failed_userCreate_size = chat_sist_os__answer__get_packed_size(&respuesta_failed_userCreate);
      uint8_t *respuesta_failed_userCreate_buf = malloc(respuesta_failed_userCreate_size);
      chat_sist_os__answer__pack(&respuesta_failed_userCreate, respuesta_failed_userCreate_buf);

      //enviar - FALTA MODIFICAR SOCK POR VALORES EN SERVIDOR
      if (send(client_sock, respuesta_failed_userCreate_buf, respuesta_failed_userCreate_size, 0) == -1) {
        perror(" > Error en send");
        exit(EXIT_FAILURE);
      }

      // Demasiados clientes conectados, cerrar la conexion del nuevo cliente
      close(client_sock);
      printf(" > Demasiados clientes conectados, la conexion del nuevo cliente fue cerrada");
    }

  }
  
  return 0;
}

void *manejar_comunicaciones(void* arg)
{
  info_cliente_ *cliente = (info_cliente_*)arg;
  int client_sockfd = cliente->sockfd;

  // Manejar comunicaciones
  while (1) {
    // Recibir mensaje del cliente
    uint8_t buf[4096];
    ssize_t bytes_recieved = recv(client_sockfd, buf, sizeof(buf), 0);
    if (bytes_recieved == -1) {
        perror("> Error en recv");
        exit(EXIT_FAILURE);
    }

    ChatSistOS__UserOption *user_option_handler = chat_sist_os__user_option__unpack(NULL, bytes_recieved, buf);
    int option = user_option_handler -> op;

    pthread_mutex_lock(&structure_mutex);
    printf("REQUEST %d RECIVED FROM CLIENT %s\n", option, cliente -> name);
    pthread_mutex_unlock(&structure_mutex);

    ChatSistOS__Answer respuesta_al_cliente = CHAT_SIST_OS__ANSWER__INIT;
    respuesta_al_cliente.op = option;

    //verificar que opcion viene el usuario.
    if (option == 0)
    {
      pthread_mutex_lock(&structure_mutex);
      printf("USER %s DISCONCTED\n", cliente -> name);
      pthread_mutex_unlock(&structure_mutex);
      pthread_exit(NULL);
      close(client_sockfd);
    }
    else if (option == 1)
    {
      ChatSistOS__Message *mensaje_recibido_general = user_option_handler->message;
      char sender_mensaje_general[101];
      strcpy(sender_mensaje_general, mensaje_recibido_general->message_sender);
      char contenido_mensaje[1024];
      strcpy(contenido_mensaje, mensaje_recibido_general->message_content);

      ChatSistOS__Message mensaje_a_enviar_general = CHAT_SIST_OS__MESSAGE__INIT;
      mensaje_a_enviar_general.message_private = 0;
      mensaje_a_enviar_general.message_sender = sender_mensaje_general; 
      mensaje_a_enviar_general.message_content = contenido_mensaje;

      //ARMAR ANSWER
      ChatSistOS__Answer respuesta_mensaje_broadcast = CHAT_SIST_OS__ANSWER__INIT;
      respuesta_mensaje_broadcast.op = 1;
      respuesta_mensaje_broadcast.response_status_code = 200;
      respuesta_mensaje_broadcast.response_message = " > Mensaje enviado con exito!";
      respuesta_mensaje_broadcast.message = &mensaje_a_enviar_general;

      //ENVIARLE AL USUARIO LA RESPUESTA
      //serializar
      size_t respuesta_mensaje_broadcast_size = chat_sist_os__answer__get_packed_size(&respuesta_mensaje_broadcast);
      uint8_t *respuesta_mensaje_broadcast_buf = malloc(respuesta_mensaje_broadcast_size);
      chat_sist_os__answer__pack(&respuesta_mensaje_broadcast, respuesta_mensaje_broadcast_buf);


      //enviar a todos los activos que no sean sender
      pthread_mutex_lock(&structure_mutex);
      for (int i = 0; i < num_clientes; i++) {
        if (clientes[i].stats == 1) {
          if (send(clientes[i].sockfd, respuesta_mensaje_broadcast_buf, respuesta_mensaje_broadcast_size, 0) == -1) {
            perror(" > Error en send");
            exit(EXIT_FAILURE);
          }
        }
      }
      pthread_mutex_unlock(&structure_mutex);

      //ARMAR ANSWER PARA SENDER
      respuesta_al_cliente.response_status_code = 200;
      respuesta_al_cliente.response_message = " > Se ha enviado el mensaje con exito.\n";
    }
    else if (option == 2)
    {
      // opcion 2

      ChatSistOS__Message *mensaje_recibido_priv = user_option_handler->message;
      char sender_mensaje_general[101];
      strcpy(sender_mensaje_general, mensaje_recibido_priv->message_sender);
      char contenido_mensaje[1024];
      strcpy(contenido_mensaje, mensaje_recibido_priv->message_content);
      char recieves_mensaje_general[101];
      strcpy(recieves_mensaje_general, mensaje_recibido_priv->message_destination);

      ChatSistOS__Message mensaje_a_enviar_priv = CHAT_SIST_OS__MESSAGE__INIT;
      mensaje_a_enviar_priv.message_private = 1;
      mensaje_a_enviar_priv.message_sender = sender_mensaje_general; 
      mensaje_a_enviar_priv.message_content = contenido_mensaje;
      mensaje_a_enviar_priv.message_destination = recieves_mensaje_general;

      //enviar a el usuario destinatario
      pthread_mutex_lock(&structure_mutex);
      int user_found = 0;
      int indice_user = 0;
      for (int i = 0; i < num_clientes; i++) {
        if (clientes[i].stats == 1 && strcmp(clientes[i].name, recieves_mensaje_general) == 0) {
          user_found++; 
          indice_user = i;
        }
      }

      if (user_found == 0) {
        // enviarle al sender que no se logro enviar a destinatario
        respuesta_al_cliente.response_status_code = 400;
        respuesta_al_cliente.response_message = " > No se ha logrado enviar el mensaje al destinatario.\n";

      } else {
        // answer de si lo encontro al reciever para que tenga el mensaje
        ChatSistOS__Answer respuesta_mensaje_priv = CHAT_SIST_OS__ANSWER__INIT;
        respuesta_mensaje_priv.op = 1;
        respuesta_mensaje_priv.response_status_code = 200;
        respuesta_mensaje_priv.response_message = " > Mensaje enviado con exito!";
        respuesta_mensaje_priv.message = &mensaje_a_enviar_priv;

        //ENVIARLE AL USUARIO LA RESPUESTA
        //serializar
        size_t respuesta_mensaje_priv_size = chat_sist_os__answer__get_packed_size(&respuesta_mensaje_priv);
        uint8_t *respuesta_mensaje_priv_buf = malloc(respuesta_mensaje_priv_size);
        chat_sist_os__answer__pack(&respuesta_mensaje_priv, respuesta_mensaje_priv_buf);

        if (send(clientes[indice_user].sockfd, respuesta_mensaje_priv_buf, respuesta_mensaje_priv_size, 0) == -1) {
          perror(" > Error en send");
          exit(EXIT_FAILURE);
        }

        // enviarle al sender que si se logro enviar a destinatario
        respuesta_al_cliente.response_status_code = 200;
        respuesta_al_cliente.response_message = " > Se ha enviado el mensaje con exito.\n";
      }

      pthread_mutex_unlock(&structure_mutex);

    }
    else if (option == 3)
    {
      // codigo para la opcion 3

      ChatSistOS__Status *handle_status = user_option_handler->status;
      char* usr_status_handler = handle_status->user_name;
      int nuevo_valor_status = handle_status->user_state;

      int cliente_index = -1; 
      pthread_mutex_lock(&structure_mutex);
      
      for (int i = 0; i < num_clientes; i++) {
        if (strcmp(clientes[i].name, usr_status_handler) == 0) {
          cliente_index = i;
          break;
        }
      }

      // Si se encontró un cliente con el nombre de usuario especificado, actualizar su estado
      if (cliente_index != -1) {
        respuesta_al_cliente.response_status_code = 200;
        clientes[cliente_index].stats = nuevo_valor_status;
        // MANDAR ANSWER QUE SI SE LOGRO
        respuesta_al_cliente.response_message = " > Se ha cambiado tu status con exito.\n";
      } else {
        // MANDAR ANSWER QUE NO SE LOGRO
        respuesta_al_cliente.response_status_code = 400;
        respuesta_al_cliente.response_message = " > ERROR. No se ha cambiado tu status. Usuario no encontrado.\n";
      }

      pthread_mutex_unlock(&structure_mutex);
    }
    else if (option == 4)
    {
      // codigo para la opcion 4 
      char usuarios[1024] = "";
      respuesta_al_cliente.response_message = "";

      pthread_mutex_lock(&structure_mutex);
      // Buscar los clientes con stats igual a 1 y agregar sus nombres de usuario al arreglo usuarios_conectados
      int num_usuarios_conectados = 0;
      for (int i = 0; i < num_clientes; i++) {
        if (clientes[i].stats == 1){
          strcat(usuarios, "> ");
          strcat(usuarios, clientes[i].name);
          strcat(usuarios, "\n");
        }
      }

      respuesta_al_cliente.response_message = usuarios;
      
      // si la lista tiene elementos se genera answer
      respuesta_al_cliente.response_status_code = 200;


      pthread_mutex_unlock(&structure_mutex);
    }
    else if  (option == 5)
    {
      ChatSistOS__UserList *usuario_especifico = user_option_handler -> userlist;
      char *user_to_see = usuario_especifico -> user_name;
      int index_case_5;
      int user_founded_5 = 0;
      info_cliente_ cliente_objetivo_5;

      // codigo para la opcion 5
      pthread_mutex_lock(&structure_mutex);
      
      for (index_case_5 = 0; index_case_5 < num_clientes; index_case_5++)
      {
        info_cliente_ cliente_actual_5 = clientes[index_case_5];
        if (strcmp(cliente_actual_5.name, user_to_see) == 0)
        {
          cliente_objetivo_5 = cliente_actual_5;
          user_founded_5++;
        }
      }
      
      if (user_founded_5 > 0)
      {
        ChatSistOS__User user_return_5 = CHAT_SIST_OS__USER__INIT;
        user_return_5.user_ip = cliente_objetivo_5.direccion_ip;
        user_return_5.user_name = cliente_objetivo_5.name;
        user_return_5.user_state = cliente_objetivo_5.stats;


        respuesta_al_cliente.user = &user_return_5;
        respuesta_al_cliente.response_status_code = 200;
      }
      else
      {
        respuesta_al_cliente.response_status_code = 400;
        respuesta_al_cliente.response_message = "No se pudo encontrar el usuario\n";
      }
      pthread_mutex_unlock(&structure_mutex);
    }

    // Enviar mensaje al cliente
    printf("RESPONSE STATUS CODE %d", respuesta_al_cliente.response_status_code);
    if (respuesta_al_cliente.response_status_code == 200)
    {
      printf("\n");
    }
    else
    {
      printf(": %s\n", respuesta_al_cliente.response_message);
    }

    //serializar
    size_t respuesta_al_cliente_size = chat_sist_os__answer__get_packed_size(&respuesta_al_cliente);
    uint8_t *respuesta_al_cliente_buf = malloc(respuesta_al_cliente_size);
    chat_sist_os__answer__pack(&respuesta_al_cliente, respuesta_al_cliente_buf);

    //enviar
    if (send(client_sockfd, respuesta_al_cliente_buf, respuesta_al_cliente_size, 0) == -1) {
      perror(" > Error en send");
      exit(EXIT_FAILURE);
    }

    printf("envio exitoso\n");
    
  }

  // Cerrar el socket del cliente
  close(client_sockfd);

  // Salir del hilo
  pthread_exit(NULL);
}