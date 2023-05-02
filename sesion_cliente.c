void* sesion_cliente(void *arg)
{
  tiempo = time(NULL);
  tm = localtime(&tiempo);
  strftime(horaMensaje, 100, "(%H:%M)", tm); // hora que se recibe el mensaje

  cli_count++;
  client_t *cli = (client_t *)arg;
  
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