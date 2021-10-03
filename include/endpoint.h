/**
 * e.h - main includes
 *
 * Endpoints with message queue and multiple queue worker threads.
 * 
 * Data And Signal - IT Solutions
 * http://www.dataandsignal.com
 * 2021
 *
 */

#ifndef ENDPOINT_H
#define ENDPOINT_H


#include <cd/cd.h>

typedef enum e_type {
	E_TYPE_UDP,
	E_TYPE_TCP,
	E_TYPE_HTTP,
	E_TYPE_HTTPS,
	E_TYPE_WEBSOCKET
} e_type_t;

struct e_s;
struct e_msg_s;
typedef void* (*e_on_msg_cb)(void *);
typedef void (*e_sig_handler_t)(int signo);

typedef struct e_s {
	e_type_t		type;
	uint16_t				port;
	struct cd_workqueue		*wq;
	uint32_t				wq_workers_n;
	char					wq_name[CD_STR_BUF_LEN];
	e_on_msg_cb	cb_on_msg;
	int						sockfd;
	struct sockaddr_in		servaddr, cliaddr;
} e_t;

int e_init(e_t* endpoint, e_type_t type);
int e_set_on_message_callback(e_t *endpoint, e_on_msg_cb cb);
int e_start(e_t *endpoint);

// User interface

typedef struct e_msg_s {
	char *data;
	size_t len;
} e_msg_t;

e_msg_t* e_msg_create(char *buf, size_t len);
void e_msg_destroy(e_msg_t **msg);
void e_msg_dctor_f(void *o);

typedef struct e_udp_s {
	e_t	base;
} e_udp_t;

typedef struct e_tcp_s {
	e_t	base;
} e_tcp_t;

e_udp_t* e_udp_create(void);
e_tcp_t* e_tcp_create(void);

void e_udp_destroy(e_udp_t** udp);
void e_tcp_destroy(e_tcp_t** tcp);

int e_udp_init(e_udp_t *udp);

int e_udp_set_port(e_udp_t *udp, uint16_t port);
int e_tcp_set_port(e_tcp_t *tcp, uint16_t port);

int e_udp_set_workqueue_name(e_udp_t *udp, const char *name);
int e_tcp_set_workqueue_name(e_tcp_t *tcp, const char *name);

int e_udp_set_workqueue_threads_n(e_udp_t *udp, uint32_t workers_n);
int e_tcp_set_workqueue_threads_n(e_tcp_t *tcp, uint32_t workers_n);

int e_udp_set_on_message_callback(e_udp_t *udp, e_on_msg_cb cb);
int e_tcp_set_on_message_callback(e_tcp_t *tcp, e_on_msg_cb cb);

int e_udp_set_signal_handler(int signo, e_sig_handler_t sig_handler);
int e_tcp_set_signal_handler(int signo, e_sig_handler_t sig_handler);

int e_udp_loop(e_udp_t *udp);
int e_tcp_loop(e_tcp_t *tcp);

int e_udp_stop(e_udp_t *udp);
int e_tcp_stop(e_tcp_t *tcp);

#define E_UDP_BUFLEN 2000

#endif // ENDPOINT_H
