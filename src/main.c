#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>

#include "server.h"


void usage(void) {
	puts(
		"among-foss: game server for Among FOSS\n"
		"Usage:\n"
		"  among-foss [OPTIONS]\n"
		"\n"
		"Options:\n"
		"  -p <PORT>               Use specified port instead of 1234\n"
		"  -h                      Show this screen and exit"
	);
}


int main(int argc, char *argv[]) {
	uint16_t port = 1234;

	int ch;
	while ((ch = getopt(argc, argv, "hp:")) != -1) {
		switch (ch) {
		case 'p':;
			uint32_t t_port;
			t_port = strtod(argv[optind - 1], NULL);
			if (errno == EINVAL) {
				fputs("Invalid port\n", stderr);
				exit(1);
			}

			if (t_port >= (2<<15)) {
				fputs("Port out of range\n", stderr);
				exit(1);
			}
			port = t_port;
			break;
		case '?':
		case 'h':
		default:
			usage();
			exit(0);
			break;
		}
	}

	/* Start the server on the specified port. */
	start_server(port);
}

