/**
 * e.c - Endpoints
 *
 * Endpoints with message queue and multiple queue worker threads.
 * 
 * Data And Signal - IT Solutions
 * http://www.dataandsignal.com
 * 2021
 *
 */

#include "../include/endpoint.h"


e_udp_t* e_udp_create(void)
{
	e_udp_t *udp = malloc(sizeof(e_udp_t));
	if (!udp)
		return NULL;
	memset(udp, 0, sizeof(e_udp_t));

	return udp;
}

void e_udp_destroy(e_udp_t **udp)
{
	if (!udp || !(*udp))
		return;
	if ((*udp)->base.wq) {
		cd_wq_workqueue_free(&(*udp)->base.wq);
	}
	free(*udp);
	*udp = NULL;
}

int e_udp_init(e_udp_t *udp)
{
	int n = 0;

	if (!udp)
		return -1;

	udp->base.wq = cd_wq_workqueue_default_create(udp->base.wq_workers_n, udp->base.wq_name);
	if (!udp->base.wq) {
		CD_LOG_ERR("Cannot create work queue");
		return -1;
	}

	n = socket(AF_INET, SOCK_DGRAM, 0);
	if (n < 0)
		return -1;

	udp->base.sockfd = n;

	bzero(&udp->base.servaddr, sizeof(udp->base.servaddr));
	udp->base.servaddr.sin_family = AF_INET;
	udp->base.servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	udp->base.servaddr.sin_port = htons(udp->base.port);

	if (bind(udp->base.sockfd, (struct sockaddr *) &udp->base.servaddr, sizeof(udp->base.servaddr)) < 0) {
		CD_LOG_ERR("Cannot bind to port %u", udp->base.port);
		return -2;
	}

	return 0;
}

e_msg_t* e_msg_create(char *buf, size_t len)
{
	e_msg_t *m = NULL;

	m = malloc(sizeof(*m));
	if (!m)
		return NULL;
	memset(m, 0, sizeof(*m));

	if (len) {
		m->data = malloc(len);
		if (!m->data) {
			free(m);
			return NULL;
		}
		memcpy(m->data, buf, len);
	}

	m->len = len;
	return m;
}

void e_msg_destroy(e_msg_t **msg)
{
	if (!msg || !(*msg))
		return;

	if ((*msg)->data) {
		free((*msg)->data);
		(*msg)->data = NULL;
	}
	free(*msg);
	*msg = NULL;
}

void e_msg_dctor_f(void *o)
{
	e_msg_t *msg = (e_msg_t*) o;
	if (!o)
		return;
	e_msg_destroy(&msg);
}

int e_udp_set_port(e_udp_t *udp, uint16_t port)
{
	if (!udp)
		return -1;

	udp->base.port = port;
	return 0;
}

int e_udp_set_workqueue_name(e_udp_t *udp, const char *name)
{
	if (!udp)
		return -1;

	strncpy(udp->base.wq_name, name, sizeof(udp->base.wq_name));
	return 0;
}

int e_udp_set_workqueue_threads_n(e_udp_t *udp, uint32_t workers_n)
{
	if (!udp)
		return -1;

	udp->base.wq_workers_n = workers_n;
	return 0;
}

int e_udp_set_on_message_callback(e_udp_t *udp, e_on_msg_cb cb)
{
	if (!udp)
		return -1;

	udp->base.cb_on_msg = cb;
	return 0;
}

int e_udp_set_signal_handler(int signo, e_sig_handler_t sig_handler)
{
	if (signal(signo, sig_handler) == SIG_ERR)
		return -1;
	return 0;
}

int e_udp_loop(e_udp_t *udp)
{
	socklen_t len = 0;
	char msg[E_UDP_BUFLEN] = { 0 };
	ssize_t bytes_n = 0;
	char buf[400] = { 0 }, portstr[100] = { 0 };

	struct cd_work *w = NULL;
	e_msg_t *m = NULL;


	if (!udp)
		return -1;

	// TODO instal signal handler
	// Signal(SIGINT, recvfrom_int);

	for ( ; ; ) {
		struct sockaddr_in *sin = NULL;

		len = sizeof(udp->base.cliaddr);
		bytes_n = recvfrom(udp->base.sockfd, msg, sizeof(msg), 0, (struct sockaddr*) &udp->base.cliaddr, &len);
		if (bytes_n < 0) {
			CD_LOG_ERR("recvfrom failed");
			return -1;
		}

		inet_ntop(AF_INET, &((struct sockaddr_in*) &udp->base.cliaddr)->sin_addr, buf, 400);
		sin = (struct sockaddr_in*) &udp->base.cliaddr;
		snprintf(portstr, sizeof(portstr), ":%d", ntohs(sin->sin_port));
		strcat(buf, portstr);

		CD_LOG_INFO("recvfrom %ld bytes (UDP) - from: %s", bytes_n, buf);

		m = e_msg_create(msg, bytes_n);
		if (!m) {
			CD_LOG_ERR("Cannot allocate memory for UDP message");
			return -1;
		}

		w = cd_wq_work_create(CD_WORK_SYNC, m, 0, udp->base.cb_on_msg, e_msg_dctor_f);
		if (!w) {
			CD_LOG_ERR("Cannot create work");
			return -1;
		}

		if (CD_ERR_OK != cd_wq_queue_work(udp->base.wq, w)) {
			CD_LOG_ERR("Cannot enqueue work");
			return -1;
		}
	}

	return 0;
}

int e_udp_stop(e_udp_t *udp)
{
	if (!udp)
		return -1;

	if (udp->base.wq) {
		if (CD_ERR_OK != cd_wq_workqueue_stop(udp->base.wq)) {
			return -1;
		}
	}

	return 0;
}
