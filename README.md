# libendpoint

Transport protocol endpoints with message queue and threaded processing, from Data And Signal's Piotr Gregor.


## Quick setup

```
git clone https://github.com/dataandsignal/libendpoint.git
cd libendpoint
make
make install
make examples
```

Tested on Debian 10 (buster):

```
image: debian-10.10.0-amd64-netinst.iso
kernel: Linux buster 4.19.0-17-amd64 #1 SMP Debian 4.19.194-3 (2021-07-18) x86_64 GNU/Linux
platform: VirtualBox
```


## Basic example

```
#include <endpoint/endpoint.h>


e_udp_t *udp = NULL;

static void* on_udp_msg(void *msg)
{
	e_msg_t *m = msg;
	printf("Got %zu bytes to process\n", m->len);
	// process m->data
	return NULL;
}

int main(void)
{
	udp = e_udp_create();
	if (!udp)
		return -1;

	e_udp_set_port(udp, 33226);
	e_udp_set_workqueue_name(udp, "UDP workqueue");
	e_udp_set_workqueue_threads_n(udp, 4);
	e_udp_set_on_message_callback(udp, on_udp_msg);
	// You can also install signal handler
	// e_udp_set_signal_handler(SIGINT, sigint_handler);

	if (e_udp_init(udp) != 0) {
		e_udp_destroy(&udp);
		return -1;
	}

	e_udp_loop(udp);

	e_udp_stop(udp);
	e_udp_destroy(&udp);

	return 0;
}
```

## Deps

- [libcd](https://github.com/dataandsignal/libcd)


## UDP endpoint

UDP endpoint is implemented with [libcd's](https://github.com/dataandsignal/libcd) work queue.

It will open UDP port, create work queue, and start processing incoming packets with configured number of worker threads, distributing work evenly to your callback.


## Examples

Example UDP is availeble in /examples folder. Compile it with 'make examples' (or make examples-debug).


Run example:

```
	./examples/build/release/e-example-udp
``` 


or (debug version):

```
	./examples/build/debug/e-example-udp
``` 


Send some data and enjoy:

```
echo -n "hello" >/dev/udp/localhost/33226
echo -n "123456" >/dev/udp/localhost/33226
```

You will see it getting processed:

```
root@ip:~/libendpoint#  ./examples/build/release/e-example-udp
2021-10-03_11-53-53:570742	INFO	src/endpoint.c:186:e_udp_loop():	recvfrom 5 bytes (UDP) - from: 127.0.0.1:41956
Got 5 bytes to process
2021-10-03_11-53-54:994748	INFO	src/endpoint.c:186:e_udp_loop():	recvfrom 6 bytes (UDP) - from: 127.0.0.1:56939
Got 6 bytes to process
```


## BUILD

This builds on Linux Debian. make, make debug and make release produce shared library in build/debug or build/release folders.

```
make debug			-> for debug library build
make release			-> for release library build
make examples-debug		-> for build and run of debug version of examples 
make examples-release		-> for build and run of release version of examples

make				-> same as make release
make examples			-> same as make examples-release

make clean			-> remove library binaries
make examples-clean		-> remove examples binaries
make clean-all			-> remove library and examples binaries

make install-headers		-> create /usr/local/include/endpoint folder and install lib headers to it
make install-debug		-> install debug version of library to /lib
make install-release		-> install release version of library to /lib
make install			-> same as make install-release

make uninstall			-> remove library from /lib and headers from /usr/local/include/endpoint
```

## Contribute

Please submit any issues [here](https://github.com/dataandsignal/libendpoint/issues).

All contributions are welcome, please submit PRs [here](https://github.com/dataandsignal/libendpoint/pulls).

Enjoy!
