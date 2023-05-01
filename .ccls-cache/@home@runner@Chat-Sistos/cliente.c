#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

int main(int arg, char **args)
{
  /*
  int listen_socket;      // para crear socket
  struct sockaddr_in servidorADDR; // para crear socket

  time_t tiempo; // hora actual
  struct tm *tm;
  char horaMensaje[100];
  char *tmp;

  // char mensaje_a_cliente[100] = ""

  // -------------------------------------- Socket
  // ----------------------------------------------
  listen_socket = socket(AF_INET, SOCK_STREAM, 0);
  bzero(&servidorADDR, sizeof(servidorADDR));
  
  servidorADDR.sin_family = AF_INET;
  servidorADDR.sin_port = *args[0];

  inet_pton(AF_INET, *args[1], &(servidorADDR.sin_addr));
  connect(
    listen_socket,
    (struct sockaddr*) &servidorADDR,
    sizeof(servidorADDR)
  );

  */

  // TODO terminar de definir la conexion con el socket
  /*
    1. Chatear con todos los usuarios (broadcasting).
    2. Enviar y recibir mensajes directos, privados, aparte del chat general.
    3. Cambiar de status.
    4. Listar los usuarios conectados al sistema de chat.
    5. Desplegar información de un usuario en particular.
    6. Ayuda.
    7. Salir.
  */
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
    scanf("%d", &opcion_menu)
    if (opcion_menu == 1)
    {
      printf("1\n");
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

  return 0;
}