# NetKit - Making Sockets in C Easy!

Have you ever spent way too much time poring over man pages, network guides and textbooks trying to figure out the C socket library? Wondering what in the heck is the difference between a `struct sockaddr`, a `struct addrinfo`, a `struct in_addr` and a `struct hostent`? Why there's `bind` and `connect` and `listen` and `accept` and `socket`? 

Meanwhile, your friend using python just gets to call `socket` or whatever, and be done with it! This library is meant to help with that. Too many times, I've found myself wanting to build something with sockets -- even something exceedingly simple -- and found myself having to go back to Beej's Network Guide (Beej has saved my ass many a time!) and figure this stuff out all over again. No longer! Now if you want to connect to `www.google.com`, you can just say:

```c
connection_t *con = nk_connect_to("www.google.com", "80");
```

And if you want to make a listening server on port 7890, you can just say

```c
connection_t *con = nk_listen_on("7890");
```

And if you want to accept a client on that port, you just call:

```c
connection_t *incoming_con = nk_accept(con);
```

So you can write a `hello, world!` server in one line:

```c
#include "../include/netkit.h"

int main(int argc, char const *argv[]) {
	nk_send(nk_accept(nk_listen_on("7890")), "hello, world!\r\n");
}
```

A `connection_t` pointer holds all of the relevant information - the file descriptor in use, the IP version (4 or 6), the IP address, the hostname and port (if set), and various options. There's even a convenient print function to display the info:

```c
connection_t *con = nk_connect_to("www.github.com", "80");
if (con)
	nk_print_connection(con);
```

which prints:
```
Connection:
	Type: Client (seeking) type
	Version: IPV4
	IP address: 204.232.175.90
	Hostname: www.github.com
	Port: 80
```

There are also versions of `send` and `recv`, including a version of `recv` which breaks on a user-specified delimiter (such as `\r\n`). Likely to come later are ability to choose TCP or UDP, simple functions for parsing/constructing HTTP requests/responses, grabbing the content of web pages, and more. There may be asyncronous functionality (using `libuv`, for example) in the future as well.

## Installation and Usage:

Note that I've only run this on Mac OS X so far, but it should work on any unix system. On windows, a savvy user might be able to hack it to work.

Run `make` from the main directory. This should put `netkit.o` in the `/bin` directory. To use it, include `/include/netkit.h` and link with `/bin/netkit.o`. To build the examples, run `make` in the examples directory.

## Examples:

* `one_liner_helloworld.c`: Does what you expect it to do.
* `mini_http.c`: A slightly more fleshed-out version of the hello world server, sends it in (somewhat) properly formed HTTP and HTML format, and does a proper teardown of the connection (calling `nk_close()`). Also prints out the request it receives from a browser (or otherwise).
* `connector.c`: Connects to a specified server and port, sends a GET request and returns a response.
* `netkit_telnet.c`: A pretty crummy version of telnet, but barely functional. Uses threads.