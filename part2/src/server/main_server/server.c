#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include "message.h"
#include <netdb.h>
#include <limits.h>

#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "dlinkedlist.c"
#include "read_line.h"

#define FALSE 0
#define TRUE 1

#define LISTEN_BACKLOG 50
#define MAX_LINE 256


pthread_mutex_t mutex_msg;
pthread_mutex_t message_id_lock;
pthread_cond_t cond_msg;
int sock_not_free = 1;

void* process_request(void* s);
int connect_user(int s_local, char* username);
int disconnect(char* username);
int unregister(char* username);
int send_message(char* sender, char* receiver, char* message);
int setMessage(char * str, char * dest);
int sendToClient(int socket, char * msg);

int sendToClient(int socket, char * msg){
    char  str [MAX_LINE];
    setMessage(msg, str);
    send(socket, str, MAX_LINE, MSG_NOSIGNAL);
    return 0;
}

int setMessage(char *str, char * dest){
    char msg[MAX_LINE];
    memset(msg, '\0', sizeof(msg));
    strcpy(msg, str);
    strcpy(dest, msg);
    return 0;
}

int main(int argc, char* argv[]) {
    char* server_ip;
    int server_port;
    int server_socket, client_socket;
    struct sockaddr_in server, client;
    struct ifreq ifr;
    socklen_t peer_addr_size = sizeof(struct sockaddr_in);

    if (argc != 3 || strcmp(argv[1], "-p") != 0) {
      fprintf(stderr, "%s\n\n", "usage: ./server -p <port>");
      return -1;
    }

    next_message_id = 1;

    server_port = atoi(argv[2]);
    server_socket = socket(AF_INET, SOCK_STREAM, 0); // Open socket

    int val;
    val = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&val, sizeof(int));

    if (server_socket == -1) {
      fprintf(stderr, "%s\n", "s> ERROR socket cannot be opened");
      return -1;
    }

    bzero((char *)&ifr, sizeof(struct ifreq));
    ifr.ifr_addr.sa_family = AF_INET; // Set family IPv4
    //snprintf(ifr.ifr_name, IFNAMSIZ, "eth0");
    snprintf(ifr.ifr_name, IFNAMSIZ, "lo");
    ioctl(server_socket, SIOCGIFADDR, &ifr);
    server_ip = inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
    /**********************************************/

    bzero((char *)&server, sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;
    if (inet_aton(server_ip, &server.sin_addr) == 0) {
      fprintf(stderr, "%s\n", "ERROR invalid IP address");
      return -1;
    }
    server.sin_port = htons(server_port);

    if (bind(server_socket, (struct sockaddr *) &server, sizeof(struct sockaddr_in)) == -1) {
      fprintf(stderr, "%s\n", "ERROR binding failed");
      return -1;
    }

    if (listen(server_socket, LISTEN_BACKLOG) == -1) {
      fprintf(stderr, "%s\n", "ERROR listening failed");
      return -1;
    }

    pthread_attr_t t_attr;
    pthread_mutex_init(&mutex_msg, NULL);
    pthread_mutex_init(&message_id_lock, NULL);
    pthread_cond_init(&cond_msg, NULL);
    pthread_attr_init(&t_attr);
    pthread_attr_setdetachstate(&t_attr, PTHREAD_CREATE_DETACHED);

    pthread_t thr;

    printf("s> init server %s:%d\n", server_ip, server_port);
    printf("s> \n");

    while(TRUE) {

      bzero((char *)&client, sizeof(struct sockaddr_in));
      client_socket = accept(server_socket, (struct sockaddr *)&client, &peer_addr_size);
      pthread_create(&thr, &t_attr, &process_request, (void *)&client_socket);

      pthread_mutex_lock(&mutex_msg);
      while(sock_not_free) {
        pthread_cond_wait(&cond_msg, &mutex_msg);
      }
      sock_not_free = TRUE;
      pthread_mutex_unlock(&mutex_msg);

    }
}

int register_user(struct sockaddr_in* client_addr, char *username) {
  if (search(username) != NULL) {
    printf("s> REGISTER %s FAIL\n", username);
    return 1;
  } else {
    struct user *new_user = (struct user*)malloc(sizeof(struct user));
    strcpy(new_user->username, username);
    new_user->status = 0;
    new_user->ip_address = &client_addr->sin_addr;
    new_user->port = ntohs(client_addr->sin_port);
    new_user->pending_messages = ConstructQueue(10);
    new_user->last_message = 0;

    Node* new_node = getNewNode(new_user);
    insert(new_node);
    printf("s> REGISTER %s OK\n", username);
  }
  return 0;
}

int process_data(int s_local, char* operation) {
  char argument1[MAX_LINE];

  if (readLine(s_local, argument1, MAX_LINE) == -1) {
    fprintf(stderr, "s> ERROR reading line\n");
    return 2;
  }

  if (operation == NULL || argument1 == NULL) {
    printf("s> ERROR MESSAGE FORMAT\n");
    return 2;
  }

  if (strcmp(operation, "REGISTER\0") == 0) {
    struct sockaddr_in client_addr;
    socklen_t size;
    size = sizeof(struct sockaddr_in);
    getpeername(s_local, (struct sockaddr *)&client_addr, &size);
    return register_user(&client_addr, argument1);

  } else if (strcmp(operation, "UNREGISTER\0") == 0) {
    return unregister(argument1);

  } else if (strcmp(operation, "CONNECT\0") == 0) {
    return connect_user(s_local, argument1);

  } else if (strcmp(operation, "DISCONNECT\0") == 0) {
    return disconnect(argument1);
  } else if (strcmp(operation, "SEND\0") == 0) {
    char argument2[MAX_LINE];
    if (readLine(s_local, argument2, MAX_LINE) == -1) {
      fprintf(stderr, "s> ERROR reading line\n");
      return 2;
    }
    char argument3[MAX_LINE];
    if (readLine(s_local, argument3, MAX_LINE) == -1) {
      fprintf(stderr, "s> ERROR reading line\n");
      return 2;
    }

    return send_message(argument1, argument2, argument3);
  } else {
    fprintf(stderr, "s> ERROR MESSAGE FORMAT");
    return 2;
  }
  return 0;
}

void* process_request(void* s) {
  int s_local;
  char return_code;
  int error;

  pthread_mutex_lock(&mutex_msg);
  s_local = *(int*)s;
  sock_not_free = FALSE;
  pthread_cond_signal(&cond_msg);
  pthread_mutex_unlock(&mutex_msg);

  char operation[MAX_LINE];
  bzero(operation, MAX_LINE);
  if (readLine(s_local, operation, MAX_LINE) == -1) {
    fprintf(stderr, "s> ERROR reading line\n");
    error = -2;
    pthread_exit(&error);
  }

  return_code = process_data(s_local, operation);

  send(s_local, &return_code, 1, MSG_NOSIGNAL);

  /* If we are sending a message, we also return to the sender the message id */
  if(strcmp(operation, "SEND\0") == 0){
    char id_toClient[MAX_LINE];
    sprintf(id_toClient, "%u", (next_message_id-1));
    sendToClient(s_local, id_toClient);
  }
  close(s_local);
  pthread_exit(0);

  return 0;
}


int connect_user(int s_local, char* username){

  struct sockaddr_in client_addr;
  socklen_t size;
  size = sizeof(struct sockaddr_in);
  getpeername(s_local, (struct sockaddr *)&client_addr, &size);

  struct Node *user_node;
  user_node = search(username);

  //User does not exist
  if(user_node == NULL){
    printf("s> CONNECT %s FAIL\n", username);
    return 1;
  }
    //User is already connected
  if(user_node->data->status != 0){
    printf("s> CONNECT %s FAIL\n", username);
    return 2;
  }
  else{ //User is disconnected

    //Filling the data for the user node to be connected
    struct user *data_connected = (struct user*) malloc(sizeof(struct user));
    strcpy(data_connected->username, username);
    data_connected->status = TRUE;
    data_connected->ip_address = &client_addr.sin_addr;

    char argument2[MAX_LINE];
    if (readLine(s_local, argument2, MAX_LINE) == -1) {
      fprintf(stderr, "s> ERROR reading line\n");
      return 2;
    }
    data_connected->port = atoi(argument2);
    data_connected->pending_messages = user_node->data->pending_messages;

    //Create the updated node with the data previously filled
    struct Node *user_connected = getNewNode(data_connected);
    //Update node
    modify(user_connected);


    if(isEmpty(user_connected->data->pending_messages) == 0){

      //Preparing the socket to provide results
      int sd;
      struct sockaddr_in receiver_client;

      char msg_id_in_char[11];
      char * send_message = "SEND_MESSAGE";

      while(isEmpty(user_connected->data->pending_messages) != 1){

        sd = socket(AF_INET, SOCK_STREAM, 0);
        if (sd == -1) {
          return 3;
        }

        bzero((char *)&receiver_client, sizeof(struct sockaddr_in));
        memcpy(&(receiver_client.sin_addr), user_connected->data->ip_address, sizeof(struct in_addr));
        receiver_client.sin_family = AF_INET;
        receiver_client.sin_port = htons(user_connected->data->port);

        if (connect(sd, (struct sockaddr *)&receiver_client, sizeof(struct sockaddr_in)) < 0) {
          return 3;
        }

        NODE *queue_message = Dequeue(user_connected->data->pending_messages);

        sprintf(msg_id_in_char, "%u", queue_message->data.mes->id);

        sendToClient(sd, send_message);
        sendToClient(sd, queue_message->data.mes->from_user);
        sendToClient(sd, msg_id_in_char);
        sendToClient(sd, queue_message->data.mes->text);

        close(sd);

        printf("s> SEND MESSAGE %d FROM %s TO %s\n", queue_message->data.mes->id, queue_message->data.mes->from_user, user_connected->data->username);
      }
    }
      printf("s> CONNECT %s OK\n", username);
  }
  return 0;
}


int disconnect(char* username){
  //stop CONNECT thread

  Node* userNode = search(username);

  /* If the user is not found inside the data structure */
  if(userNode == NULL){
    printf("s> DISCONNECT %s FAIL\n", username);
    return 1;
  }

  /* If the user is not connected */
  if(userNode->data->status == FALSE){
    printf("s> DISCONNECT %s FAIL\n", username);
    return 2;
  }

  /* If the user is connected */
  if(userNode->data->status == TRUE){
    userNode->data->status = FALSE;
    userNode->data->ip_address = NULL;
    userNode->data->port = 0;

    /* Modify the user node on the data structure */
    if(modify(userNode) < 0){
      printf("s> DISCONNECT %s FAIL\n", username);
      return 3;
    }
    printf("s> DISCONNECT %s OK\n", username);
    return 0;
  }

  /* Error case */
  printf("s> DISCONNECT %s FAIL\n", username);
  return 3;
}

int unregister(char* username){
  /* Succesful unregister */
  if(delete(username) == 0){
    printf("s> UNREGISTER %s OK\n", username);
    return 0;
  }
  /* ERROR: user is not found in the data structure */
  else if(delete(username) == -1){
    printf("s> UNREGISTER %s FAIL\n", username);
    return 1;
  }
  /* ERROR: any other case */
  else{
    printf("s> UNREGISTER %s FAIL\n", username);
    return 2;
  }
}

int send_message(char* sender, char* receiver, char* message){

    /* The message exceeds the maximum size */
    if((strlen(message)+1) > MAX_LINE){
      return 2;
    }

    /* Find the two users inside the list */
    Node* senderNode = search(sender);
    Node* receiverNode = search(receiver);

    /* If one of the users is not found inside the data structure */
    if((senderNode == NULL) || (receiverNode == NULL)){
      return 1;
    }

    /* If the sender is disconnected */
    if(senderNode->data->status == FALSE){
      return 2;
    }

    NODE *receiver_message = malloc(sizeof(NODE));
    receiver_message->data.mes = (struct message*)malloc(sizeof(struct message));
    pthread_mutex_lock(&message_id_lock);
    receiver_message->data.mes->id = next_message_id;
    /* If we exceed the maximum size for an unsigned int*/
    if(next_message_id + 1 > UINT_MAX){
      next_message_id = 0;
    } else {
      next_message_id++;
    }
    pthread_mutex_unlock(&message_id_lock);

    senderNode->data->last_message = receiver_message->data.mes->id;

    strcpy(receiver_message->data.mes->from_user, sender);
    strcpy(receiver_message->data.mes->to_user, receiver);
    strcpy(receiver_message->data.mes->text, message);

    // printf("Sender from: %s\n", receiver_message->data.mes->from_user);
    // printf("Sender to: %s\n", receiver_message->data.mes->to_user);
    // printf("Sender mes: %s\n", receiver_message->data.mes->text);

    /* If the receiver is disconnected */
    if(receiverNode->data->status == FALSE){
      /* Put the message in the message queue */
      Enqueue(search(receiver)->data->pending_messages, receiver_message);
      printf("s> MESSAGE %u FROM %s TO %s STORED\n", receiver_message->data.mes->id, sender, receiver);
      return 0;
    } else {

      int sd;
      struct sockaddr_in receiver_client;

      sd = socket(AF_INET, SOCK_STREAM, 0);
      if (sd == -1) {
        /* fprintf(stderr, "%s\n", "s> Could not create socket");*/
        return 3;
      }

      bzero((char *)&receiver_client, sizeof(struct sockaddr_in));
      memcpy(&(receiver_client.sin_addr), receiverNode->data->ip_address, sizeof(struct in_addr));
      receiver_client.sin_family = AF_INET;
      receiver_client.sin_port = htons(receiverNode->data->port);

      if (connect(sd, (struct sockaddr *)&receiver_client, sizeof(struct sockaddr_in)) < 0) {
        return 3;
      }

      char msg_id_in_char[11];
      char * send_message = "SEND_MESSAGE";
      sprintf(msg_id_in_char, "%u", receiver_message->data.mes->id);

      sendToClient(sd, send_message);
      sendToClient(sd, senderNode->data->username);
      sendToClient(sd, msg_id_in_char);
      sendToClient(sd, receiver_message->data.mes->text);

      printf("s> SEND MESSAGE %d FROM %s TO %s\n", receiver_message->data.mes->id, sender, receiver);
    }
    return 0;
}
