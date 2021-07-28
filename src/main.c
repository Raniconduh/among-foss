#include "stdio.h"
#include "stdlib.h"
#include "stdint.h"

#include "server.h"


int main(int argc, char *argv[]) {
	uint16_t port = 1234;

	/* TODO: Parse options, port and help option */

	/* Start the server on the specified port. */
	start_server(port);
}

