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
#include <pthread.h>

#define LIMITE_SESIONES 10
#define BUFFER_SZ 2048
#define UsernameLenght 32

static int conteo_clientes = 0;

// Estructura para crear clientes
typedef struct {
  struct sockaddr_in clienteADDR;
  long sockfd;
  int stats;
  char* name;
  char* direccion_ip;
} info_cliente_;

void* manejar_comunicaciones(void* arg);


int main(int argc, char **argv)
{

  // pthreads
  // pthread_t pthread_id;

  // Ruta para coneccion
  int puerto = atoi(argv[1]);
  char *ip_server = "127.0.0.1";

  // Crear un socket para escuchar conexiones entrantes
  int server_sock = socket(AF_INET, SOCK_STREAM, 0);
  if (server_sock == -1) {
      printf(" > Error al crear el socket");
      return 1;
  }

  // Especificar la dirección IP y el puerto del servidor
  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(ip_server);
  server_addr.sin_port = htons(puerto);

  // Vincular el socket con la dirección IP y el puerto
  if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
      printf(" > Error al vincular el socket");
      return 1;
  }

  // Esperar conexiones entrantes
  if (listen(server_sock, 1) < 0) {
    printf(" > Error al esperar conexiones entrantes");
    return 1;
  }

  // Lista de clientes conectados al servidor
  info_cliente_ clientes[LIMITE_SESIONES];
  int num_clientes = 0;

  // Aceptar conexiones entrantes
  while (1) {
    // Aceptar la conexión entrante
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_len);
    if (client_sock < 0) {
      printf(" > Error al aceptar la conexión entrante");
      continue;
    }

    // AQUI RECIBE EL USER OPTION PARA CREAR USUARIO

    // Verificar si hay menos de MAX_CLIENTES clientes conectados actualmente
    if (num_clientes < LIMITE_SESIONES) {
      // Agregar el nuevo cliente a la lista de clientes conectados
      clientes[num_clientes].sockfd = client_sock;
      clientes[num_clientes].clienteADDR = client_addr;
      clientes[num_clientes].stats = 1;      
      clientes[num_clientes].name = NULL; //cambiar por el del usuario
      clientes[num_clientes].direccion_ip = NULL; //cambiar por el del usuario

      num_clientes++; // suma al contador de clientes

      // Crear un hilo para manejar conexiones con ese cliente
      pthread_t hilo;
      if (pthread_create(&hilo, NULL, manejar_comunicaciones, (void*)&clientes[num_clientes-1]) < 0) {
        printf(" > Error al crear un nuevo hilo para manejar las comunicaciones con el cliente");
        continue;
      }

      // ARMAR ANSWER PARA ENVIAR Y DECIR SI SE LOGRO CREAR USER


    } else {

      // ARMAR ANSWER PARA ENVIAR Y DECIR NO SE LOGRO CREAR USER

      // Demasiados clientes conectados, cerrar la conexión del nuevo cliente
      close(client_sock);
      printf(" > Demasiados clientes conectados, la conexión del nuevo cliente fue cerrada");
    }

  ChatSistOS__Answer respuesta = CHAT_SIST_OS__ANSWER__INIT;
  respuesta.op = 1; // Opción de enviar un mensaje

  // Serializar la estructura de UserOption para enviarla al servidor
  uint8_t buffer[1024];
  unsigned int length = user_option__get_packed_size(&userOption);
  user_option__pack(&userOption, buffer);

  // se deberia de recibir el useroption AQUI
    /*
    Recibo user option
    
    desempaqueto

    deserializo

    saco el op del user option

    if op == 1
    elif op == 2
    elif op == 3
    */

  // Enviar el mensaje al servidor
  if (send(sock, buffer, length, 0) < 0) {
      printf("Error al enviar el mensaje");
      return 1;
  }

  // Recibir y enviar mensajes con el cliente
  char buffer[1024];
  int bytes_received;
  while ((bytes_received = recv(client_sock, buffer, sizeof(buffer), 0)) > 0) {
      printf("Mensaje recibido: %s", buffer);

      // Enviar una respuesta al cliente
      char respuesta[] = "Mensaje recibido correctamente";
      send(client_sock, respuesta, strlen(respuesta), 0);
  }

  // Eliminar la estructura del cliente de la lista de
  for (int i = 0; i < num_clientes; i++) {
    if (clientes[i].sockfd == client_sock) {
      // Mover todas las estructuras siguientes una posición hacia atrás en el arreglo
      for (int j = i; j < num_clientes - 1; j++) {
        clientes[j] = clientes[j+1];
      }
      // Disminuir el contador de num_clientes en 1
      num_clientes--;
      break;
    }
  }

  // Cerrar los sockets
  close(client_sock);
  close(server_sock);

  return 0;
}

/*
// Aceptar conexiones de clientes entrantes
while (1) {
    // Esperar por una conexión entrante
    int client_sock = accept(sock, (struct sockaddr *)&client, (socklen_t*)&c);
    if (client_sock < 0) {
        printf("Error al aceptar la conexión del cliente");
        continue;
    }

    // Verificar si hay menos de MAX_CLIENTES clientes conectados actualmente
    if (num_clientes < MAX_CLIENTES) {
        // Agregar el nuevo cliente a la lista de clientes conectados
        clientes[num_clientes].sockfd = client_sock;
        clientes[num_clientes].clienteADDR = client;
        clientes[num_clientes].stats = 1;
        clientes[num_clientes].name = NULL;
        num_clientes++;

        // Crear un hilo para manejar las comunicaciones con el nuevo cliente
        pthread_t hilo;
        if (pthread_create(&hilo, NULL, manejar_comunicaciones, (void*)&client_sock) < 0) {
            printf("Error al crear un nuevo hilo para manejar las comunicaciones con el cliente");
            continue;
        }
    } else {
        // Demasiados clientes conectados, cerrar la conexión del nuevo cliente
        close(client_sock);
        printf("Demasiados clientes conectados, la conexión del nuevo cliente fue cerrada");
    }
}
*/



/*
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

  
  struct sockaddr_in clienteADDR; // para conectar sockets
  int clientesActivos = 0;
  while (1) {

    // recoger los clientes de la lista de espera
    clientes_en_espera = accept(listen_socket, (struct sockaddr *)clienteADDR, sizeof(clienteADDR));

    // Verificar que no exceda limites de servidor
    if(clientesActivos == LIMITE_SESIONES){
        printf(" > Error SERVER CLIENTES MAXED OUT: ");
        print_client_addr(clienteADDR);
        printf(":%d\n", clienteADDR.sin_port);
        close(clientes_en_espera);
        continue;
    }

    // Una vez verificado que el cliente a agregar no excede la cantidad maxima entonces se registra
    info_cliente_ *nuevo_cliente = (info_cliente_ *)malloc(sizeof(info_cliente_));
    nuevo_cliente->address = cli_addr;
    nuevo_cliente->sockfd = connfd;
    nuevo_cliente->uid = uid++;
    nuevo_cliente->status = 1;

    // Add client to the queue and fork thread
    queue_add(cli);
    pthread_create(&tid, NULL, &handle_client, (void*)cli);
    
    bzero(cadena, 100); // limpiar cadena

    if (read(clientes_en_espera, cadena, 100) < 0) {
      return -1;
    }
    
  }
  return 0;
}
*/

