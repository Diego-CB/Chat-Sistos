#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <signal.h>

#define LIMITE_SESIONES 5
#define BUFFER_SZ 2048
#define UsernameLenght 32

static int conteo_clientes = 0;

// Estructura para crear clientes
typedef struct {
  struct sockaddr_in clienteADDR;
  long socket_cliente;
  int estatus_cliente;
  std::string name;

  // Constructor de la estructura
  info_cliente_() {
    name.resize(UsernameLenght);
  }
} info_cliente_;

info_cliente_ *clients[clientesMaximo]; // array de clientes
pthread_mutex_t threadMutex_infoCliente = PTHREAD_MUTEX_INITIALIZER;

void str_trim_lf(char *arr, int length) {
  int i;
  for (i = 0; i < length; i++) {
    if (arr[i] == '\n') {
      arr[i] = '\0';
      break;
    }
  }
}

void queue_add(info_cliente_ *cl) {
  pthread_mutex_lock(&threadMutex_infoCliente);

  for (int i = 0; i < clientesMaximo; i++) {
    if (!clients[i]) {
      clients[i] = cl;
      break;
    }
  }

  pthread_mutex_unlock(&threadMutex_infoCliente);
}

void queue_remove(int userID) {
    pthread_mutex_lock(&threadMutex_infoCliente);

    for (int i = 0; i < clientesMaximo; i++) {
        if (clients[i]) {
            if (clients[i]->userID == userID) {
                clients[i] = NULL;
                printf("Client disconnected: %d\n", userID);
                break;
            }
        }
    }

    pthread_mutex_unlock(&threadMutex_infoCliente);
}

int get_userID_by_name(char *name) {
    for (int i = 0; i < clientesMaximo; i++) {
        if (clients[i]) {
            if (strcmp(clients[i]->name, name) == 0) {
                return clients[i]->userID;
            }
        }
    }
    return -1;
}

void show_users(int userID) {
    pthread_mutex_lock(&threadMutex_infoCliente);

    // get the name of all the users
    // that do not have the same userID as the current user
    // and assign it to the variable users
    char users[BUFFER_SZ] = {};

    strcat(users, "********************\n");
    strcat(users, "Users:\n");

    for (int i = 0; i < clientesMaximo; i++) {
        if (clients[i]) {
            if (clients[i]->userID != userID) {
                strcat(users, clients[i]->name);
                strcat(users, "\n");
            }
        }
    }

    strcat(users, "********************\n");
    pthread_mutex_unlock(&threadMutex_infoCliente);

    // Send the user with given userID the list of users
    send_message_to_specific_client(users, userID, userID);
}

void send_message(char *s, int userID) {
    pthread_mutex_lock(&threadMutex_infoCliente);

    for (int i = 0; i < clientesMaximo; i++) {
        if (clients[i]) {
            if (clients[i]->userID != userID) {
                if (write(clients[i]->socket_cliente, s, strlen(s)) < 0) {
                    perror("ERROR: write to descriptor failed");
                    break;
                }
            }
        }
    }

    pthread_mutex_unlock(&threadMutex_infoCliente);
}

//---------------------------------------------------------------------------------------------
void send_message_to_specific_client(char *s, int userID, int userID_to){
    pthread_mutex_lock(&threadMutex_infoCliente);

    for(int i = 0; i < clientesMaximo; i++){
        if(clients[i]){
            if(clients[i]->userID == userID_to){
                if(write(clients[i]->socket_cliente, s, strlen(s)) < 0){
                    perror("ERROR: write to descriptor failed");
                    break;
                }
            }
        }
    }

    pthread_mutex_unlock(&threadMutex_infoCliente);
}

void print_client_addr(struct sockaddr_in addr){
    printf("%d.%d.%d.%d",
        addr.sin_addr.s_addr & 0xff,
        (addr.sin_addr.s_addr & 0xff00) >> 8,
        (addr.sin_addr.s_addr & 0xff0000) >> 16,
        (addr.sin_addr.s_addr & 0xff000000) >> 24);
}

void *handle_client(void *arg){
    char buff_out[BUFFER_SZ];
    char name[UsernameLenght];
    int leave_flag = 0;

    conteo_clientes++;
    info_cliente_ *cli = (info_cliente_ *)arg;

    // Name
    if(recv(cli->socket_cliente, name, UsernameLenght, 0) <= 0 || strlen(name) <  2 || strlen(name) >= UsernameLenght - 1){
        printf("Didn't enter the name.\n");
        leave_flag = 1;
    }else{
        strcpy(cli->name, name);
        sprintf(buff_out, "%s has joined\n", cli->name);
        printf("%s", buff_out);
        send_message(buff_out, cli->userID);
    }

    ChatSistOS__UserOption
    bzero(buff_out, BUFFER_SZ);

    while(1){
        if(leave_flag){
            break;
        }

        int bits_recv = recv(cli->socket_cliente, buff_out, BUFFER_SZ, 0);

        if (recieve < 0){
          continue;
        }
        
        if (buff_out < 0){
          continue;
        }
      
        if(bits_recv > 0){
            if(strlen(buff_out) > 0){

                // split the string by $@$
                char *token = strtok(buff_out, "$@$");
                
                // first token is the message
                char *msg = token;

                // second token is the userID_to
                token = strtok(NULL, "$@$");
                char *opcion = token;

                // if opcion is 1, then send to all
                if(strcmp(opcion, "1") == 0){
                    char newMessage[BUFFER_SZ] = {};

                    strcat(newMessage, "********************\n");
                    strcat(newMessage, cli->name);
                    strcat(newMessage, ": ");
                    strcat(newMessage, msg);
                    strcat(newMessage, "\n");
                    strcat(newMessage, "********************\n");


                    send_message(newMessage, cli->userID);
                    str_trim_lf(buff_out, strlen(buff_out));
                    printf("%s -> %s\n", buff_out, cli->name);
                }else if(strcmp(opcion, "2") == 0){
                    char *name = strtok(NULL, "$@$");
                    int userID_to = get_userID_by_name(name);
                    if(userID_to != -1){

                        char newMessage[BUFFER_SZ] = {};

                        strcat(newMessage, "********************\n");
                        strcat(newMessage, "Private message from ");
                        strcat(newMessage, cli->name);
                        strcat(newMessage, "\n");
                        strcat(newMessage, msg);
                        strcat(newMessage, "\n");
                        strcat(newMessage, "********************\n");

                        send_message_to_specific_client(newMessage, cli->userID, userID_to);
                        str_trim_lf(buff_out, strlen(buff_out));
                        printf("%s -> %s\n", buff_out, cli->name);
                    }
                    else{
                        char error[BUFFER_SZ] = {};
                        strcat(error, "********************\n");
                        strcat(error, "ERROR: User not found\n");
                        strcat(error, "********************\n");
                        send_message_to_specific_client(error, cli->userID, cli->userID);
                    }

                }else if(strcmp(opcion, "3") == 0){
                    // print users  
                    printf("Showing users\n");
                    show_users(cli->userID);

                }
            

            }
        }else if(bits_recv == 0 || strcmp(buff_out, "exit") == 0){
            sprintf(buff_out, "%s has left\n", cli->name);
            printf("%s", buff_out);
            send_message(buff_out, cli->userID);
            leave_flag = 1;
        }else{
            printf("ERROR: -1\n");
            leave_flag = 1;
        }

        bzero(buff_out, BUFFER_SZ);
    }

    // Delete client from queue and yield thread
    close(cli->socket_cliente);
    queue_remove(cli->userID);
    free(cli);
    conteo_clientes--;
    pthread_detach(pthread_self());

    return NULL;
}

int main(int argc, char **argv){
    if(argc != 2){
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

    if(setsockopt(listenfd, SOL_SOCKET, (SO_REUSEPORT | SO_REUSEADDR), (char*)&option, sizeof(option)) < 0){
        perror("ERROR: setsockopt failed");
        return EXIT_FAILURE;
    }

    // Bind
    if(bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
        perror("ERROR: Socket binding failed");
        return EXIT_FAILURE;
    }

    // Listen
    if(listen(listenfd, 10) < 0){
        perror("ERROR: Socket listening failed");
        return EXIT_FAILURE;
    }

    printf("=============================\n");
    printf("\tCHATROOM el buen samaritano\n");
    printf("=============================\n");


    while(1){
        socklen_t clilen = sizeof(cli_addr);
        connfd = accept(listenfd, (struct sockaddr*)&cli_addr, &clilen);

        // Check for max clients
        if((conteo_clientes + 1) == clientesMaximo){
            printf("Max clients reached. Rejected: ");
            print_client_addr(cli_addr);
            printf(":%d\n", cli_addr.sin_port);
            close(connfd);
            continue;
        }

        // Client settings
        info_cliente_ *cli = (info_cliente_ *)malloc(sizeof(info_cliente_));
        cli->address = cli_addr;
        cli->socket_cliente = connfd;
        cli->userID = userID++;
        cli->status = 1;

        // Add client to the queue and fork thread
        queue_add(cli);
        pthread_create(&tid, NULL, &handle_client, (void*)cli);

        // Reduce CPU usage
        sleep(1);


    }


    return EXIT_SUCCESS;


}