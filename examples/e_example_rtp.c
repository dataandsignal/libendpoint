/**
 * e_example_udp.c - Example UDP endpoint with workqueue
 *
 * Example UDP endpoint with message queue threaded processing.
 * 
 * Data And Signal - IT Solutions
 * http://www.dataandsignal.com
 * 2021
 *
 */

#include <endpoint/endpoint.h>


e_udp_t *udp = NULL;

static void* on_udp_msg(void *msg)
{
	e_msg_t *m = msg;

	if (!m)
		return NULL;

	printf("Got %zu bytes to process\n", m->len);

	if (m->len > 8 + 8 + 4) {
		uint32_t *p = (uint32_t*)(((char*) m->data) + 8);
		printf("SSRC: %u\n", ntohl(*p));
	}
	return NULL;
}

static void sigint_handler(int signo)
{
	printf("Signal %d (%s) received\n", signo, strsignal(signo));

	if (SIGINT == signo) {

		printf("Do widzenia!\n");

		e_udp_stop(udp);
		e_udp_destroy(&udp);
		exit(EXIT_SUCCESS);
	} else {
		printf ("Ignored\n");
	}
}

int main(void)
{
	/**
	 * By default all log from libcd goes into stderr, so you can redirect it wherever you want.
	 * If however all log output from lib should be redirected to some file then use this
	 *
	 * if (-1 == cd_util_openlog("/tmp/", "libcd"))
	 *     return -1;
	 *
	 * and all log will be there. You can also print to that file with CD_LOG_INFO/ERR/PERR/ALERT/WARN/CRIT:
	 *     CD_LOG_INFO("This goes wherever stdout/stderr output from libcd goes");
	 */

	udp = e_udp_create();
	if (!udp)
		return -1;

	e_udp_set_port(udp, 4900);
	e_udp_set_workqueue_name(udp, "UDP workqueue");
	e_udp_set_workqueue_threads_n(udp, 4);
	e_udp_set_on_message_callback(udp, on_udp_msg);
	e_udp_set_signal_handler(SIGINT, sigint_handler);

	if (e_udp_init(udp) != 0) {
		// Maybe could not bind or create a socket
		e_udp_destroy(&udp);
		return -1;
	}

	// This will block and process messages (send SIGINT to stop the endpoint and exit)
	e_udp_loop(udp);

	return 0;
}
