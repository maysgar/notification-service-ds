/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#include <memory.h> /* for memset */
#include "user_storage.h"

/* Default timeout can be changed using clnt_control() */
static struct timeval TIMEOUT = { 25, 0 };

enum clnt_stat 
init_1(int *clnt_res, CLIENT *clnt)
{
	 return (clnt_call (clnt, init, (xdrproc_t) xdr_void, (caddr_t) NULL,
		(xdrproc_t) xdr_int, (caddr_t) clnt_res,
		TIMEOUT));

}

enum clnt_stat 
register_user_1(char *username, int *clnt_res,  CLIENT *clnt)
{
	return (clnt_call(clnt, register_user,
		(xdrproc_t) xdr_char, (caddr_t) &username,
		(xdrproc_t) xdr_int, (caddr_t) clnt_res,
		TIMEOUT));
}

enum clnt_stat 
unregister_user_1(char *username, int *clnt_res,  CLIENT *clnt)
{
	return (clnt_call(clnt, unregister_user,
		(xdrproc_t) xdr_char, (caddr_t) &username,
		(xdrproc_t) xdr_int, (caddr_t) clnt_res,
		TIMEOUT));
}

enum clnt_stat 
get_user_1(char *username, struct user *usr, int *clnt_res,  CLIENT *clnt)
{
	get_user_1_argument arg;
	arg.username = username;
	arg.usr = usr;
	return (clnt_call (clnt, get_user, (xdrproc_t) xdr_get_user_1_argument, (caddr_t) &arg,
		(xdrproc_t) xdr_int, (caddr_t) clnt_res,
		TIMEOUT));
}

enum clnt_stat 
add_message_1(struct message msg, int *clnt_res,  CLIENT *clnt)
{
	return (clnt_call(clnt, add_message,
		(xdrproc_t) xdr_message, (caddr_t) &msg,
		(xdrproc_t) xdr_int, (caddr_t) clnt_res,
		TIMEOUT));
}

enum clnt_stat 
get_total_messages_1(char *username, int *clnt_res,  CLIENT *clnt)
{
	return (clnt_call(clnt, get_total_messages,
		(xdrproc_t) xdr_char, (caddr_t) &username,
		(xdrproc_t) xdr_int, (caddr_t) clnt_res,
		TIMEOUT));
}

enum clnt_stat 
get_message_1(char *username, u_int msg_id, char *md5, struct message *msg, int *clnt_res,  CLIENT *clnt)
{
	get_message_1_argument arg;
	arg.username = username;
	arg.msg_id = msg_id;
	arg.md5 = md5;
	arg.msg = msg;
	return (clnt_call (clnt, get_message, (xdrproc_t) xdr_get_message_1_argument, (caddr_t) &arg,
		(xdrproc_t) xdr_int, (caddr_t) clnt_res,
		TIMEOUT));
}
