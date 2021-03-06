/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#ifndef _USER_STORAGE_H_RPCGEN
#define _USER_STORAGE_H_RPCGEN

#include <rpc/rpc.h>


#ifdef __cplusplus
extern "C" {
#endif


struct DATA {
	struct message *mes;
};
typedef struct DATA DATA;

struct NODE {
	DATA data;
	struct NODE *prev;
};
typedef struct NODE NODE;

struct Queue {
	NODE *head;
	NODE *tail;
	int size;
	int limit;
};
typedef struct Queue Queue;

struct message {
	u_int id;
	char md5[32];
	char from_user[256];
	char to_user[256];
	char text[256];
	char filename[256];
};
typedef struct message message;

struct user {
	char username[256];
	int status;
	int ip_address;
	int port;
	Queue *pending_messages;
	u_int last_message;
};
typedef struct user user;

struct get_message_1_argument {
	char *username;
	u_int msg_id;
};
typedef struct get_message_1_argument get_message_1_argument;

#define USERSTORAGE 98
#define USERSTORAGEVER 1

#if defined(__STDC__) || defined(__cplusplus)
#define init 1
extern  int * init_1(CLIENT *);
extern  int * init_1_svc(struct svc_req *);
#define register_user 2
extern  int * register_user_1(char *, CLIENT *);
extern  int * register_user_1_svc(char *, struct svc_req *);
#define unregister_user 3
extern  int * unregister_user_1(char *, CLIENT *);
extern  int * unregister_user_1_svc(char *, struct svc_req *);
#define add_user 4
extern  int * add_user_1(struct user , CLIENT *);
extern  int * add_user_1_svc(struct user , struct svc_req *);
#define get_user 5
extern  struct user * get_user_1(char *, CLIENT *);
extern  struct user * get_user_1_svc(char *, struct svc_req *);
#define add_message 6
extern  int * add_message_1(struct message , CLIENT *);
extern  int * add_message_1_svc(struct message , struct svc_req *);
#define get_total_messages 7
extern  int * get_total_messages_1(char *, CLIENT *);
extern  int * get_total_messages_1_svc(char *, struct svc_req *);
#define get_message 8
extern  struct message * get_message_1(char *, u_int , CLIENT *);
extern  struct message * get_message_1_svc(char *, u_int , struct svc_req *);
extern int userstorage_1_freeresult (SVCXPRT *, xdrproc_t, caddr_t);

#else /* K&R C */
#define init 1
extern  int * init_1();
extern  int * init_1_svc();
#define register_user 2
extern  int * register_user_1();
extern  int * register_user_1_svc();
#define unregister_user 3
extern  int * unregister_user_1();
extern  int * unregister_user_1_svc();
#define add_user 4
extern  int * add_user_1();
extern  int * add_user_1_svc();
#define get_user 5
extern  struct user * get_user_1();
extern  struct user * get_user_1_svc();
#define add_message 6
extern  int * add_message_1();
extern  int * add_message_1_svc();
#define get_total_messages 7
extern  int * get_total_messages_1();
extern  int * get_total_messages_1_svc();
#define get_message 8
extern  struct message * get_message_1();
extern  struct message * get_message_1_svc();
extern int userstorage_1_freeresult ();
#endif /* K&R C */

/* the xdr functions */

#if defined(__STDC__) || defined(__cplusplus)
extern  bool_t xdr_DATA (XDR *, DATA*);
extern  bool_t xdr_NODE (XDR *, NODE*);
extern  bool_t xdr_Queue (XDR *, Queue*);
extern  bool_t xdr_message (XDR *, message*);
extern  bool_t xdr_user (XDR *, user*);
extern  bool_t xdr_get_message_1_argument (XDR *, get_message_1_argument*);

#else /* K&R C */
extern bool_t xdr_DATA ();
extern bool_t xdr_NODE ();
extern bool_t xdr_Queue ();
extern bool_t xdr_message ();
extern bool_t xdr_user ();
extern bool_t xdr_get_message_1_argument ();

#endif /* K&R C */

#ifdef __cplusplus
}
#endif

#endif /* !_USER_STORAGE_H_RPCGEN */
