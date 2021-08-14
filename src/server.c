#include "server.h"


struct player players[NUM_PLAYERS];

fd_set rfds, afds;


void send_data(int fd, char *format, ...) {
	char buf[1024];
	va_list args;

	va_start(args, format);
	vsnprintf(buf, sizeof(buf), format, args);
	va_end(args);

	write(fd, buf, strlen(buf));
}

void send_json_data(int fd, json_object *object) {
	char *data = convert_json_object_to_string(object);
	send_data(fd, "%s\n", data);
}

void broadcast(int sender_fd, char *format, ...) {
	char buf[1024];
	va_list args;
	int pid;

	va_start(args, format);
	vsnprintf(buf, sizeof(buf), format, args);
	va_end(args);

	printf("*> %s", buf);

	for (pid = 0; pid < NUM_PLAYERS; pid++) {
		if (players[pid].fd == -1 || players[pid].fd == sender_fd
				|| players[pid].stage == PLAYER_STAGE_NAME)
			continue;

		write(players[pid].fd, buf, strlen(buf));
	}
}

void broadcast_json(int sender_fd, json_object *object) {
	for (int pid = 0; pid < NUM_PLAYERS; pid++) {
		if (players[pid].fd == -1 || players[pid].fd == sender_fd
				|| players[pid].stage == PLAYER_STAGE_NAME)
			continue;

		send_json_data(players[pid].fd, object);
	}
}


void start_server(uint16_t port) {
	int listen_fd, new_fd, i;

	socklen_t client_size;
	struct sockaddr_in listen_addr = {0}, client_addr = {0};

	/* Initialize the player list with no file descriptors. */
	for (i = 0; i < NUM_PLAYERS; i++)
		players[i].fd = -1;

	/* Use the current time as a random seed. */
	srand((unsigned) time(NULL));

	/* Try to create a socket. */
	if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		fprintf(stderr, "Failed to create a socket.\n");
		exit(EXIT_FAILURE);
	}

	i = 1;

	if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(i))) {
		fprintf(stderr, "Failed to set socket options.\n");
		exit(EXIT_FAILURE);
	}

	listen_addr.sin_family = AF_INET;
	listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	listen_addr.sin_port = htons(port);

	if (bind(listen_fd, (struct sockaddr *)&listen_addr, sizeof(listen_addr)) < 0) {
		fprintf(stderr, "Failed to bind address.\n");
		exit(EXIT_FAILURE);
	}

	listen(listen_fd, 5);
	printf("Listening on port %d.\n", port);

	state.stage = STAGE_LOBBY;

	FD_ZERO(&afds);
	FD_SET(listen_fd, &afds);
	
	/* Handle clients indefinitely. */
	while (1) {
		rfds = afds;

		/* Check if the client is ready for writing and reading. */
		if (select(FD_SETSIZE, &rfds, NULL, NULL, NULL) < 0) {
			fprintf(stderr, "Failed to check if the client is ready.\n");
			exit(EXIT_FAILURE);
		}

		for (i = 0; i < FD_SETSIZE; ++i) {
			if (FD_ISSET(i, &rfds)) {
				if (i == listen_fd) {
					client_size = sizeof(client_addr);
					new_fd = accept(i, (struct sockaddr *)&client_addr, &client_size);

					if (new_fd < 0) {
						fprintf(stderr, "Failed to accept an incoming connection.\n");
						exit(EXIT_FAILURE);
					}

					printf("New connection from '%s:%d'.\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

					FD_SET(new_fd, &afds);

					if (welcome_client(new_fd) < 0)
						FD_CLR(new_fd, &afds);
				} else {
					if (handle_input(i) < 0) {
						close(i);
						FD_CLR(i, &afds);
					}
				}
			}
		}
	}
}
