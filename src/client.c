#include "server.h"
#include "client.h"


/* Get a player ID using the file descriptor. */
int get_pid_by_fd(int fd) {
	for (int pid = 0; pid < NUM_PLAYERS; pid++)
		if (players[pid].fd == fd)
			return pid;

	/* If no player could be found, return a negative player ID. */
	return -1;
}

/* Greet the client and ask them for their name. */
int welcome_client(int fd) {
	for (int i = 0; i < sizeof(players); i++) {
		if (players[i].fd > 0)
			continue;

		send_json_data(fd, JSON_INFO);

		//if (state.stage != STAGE_LOBBY)
			//send_data(fd, MSG_GAME_IN_PROGRESS);

		players[i].fd = fd;
		players[i].stage = PLAYER_STAGE_NAME;

		printf("Assigned player to ID %d\n", i);

		return 0;
	}

	/* Tell the client that the game is full and close the file descriptor. */
	send_json_data(fd, JSON_KICK(JSON_KICK_GAME_FULL));
	close(fd);

	return -1;
}

/* Clean the player's info. */
void disconnect_client(int fd, int pid, int should_broadcast) {
	players[pid].fd = -1;

	if (should_broadcast)
		broadcast_json(-1, JSON_LEAVE(players[pid].name));

	players[pid].name[0] = '\0';
}

/* Handle client input. */
int handle_input(int fd) {
	char input[200] = {0};
	struct json_object *parsed_input, *arg;

	int pid = get_pid_by_fd(fd);
	size_t len;

	/* Get the input. */
	len = read(fd, input, 199);

	/* If the client sends an invalid length, disconnect them. */
	if (len < 0) {
		printf("Read error from player %d\n", pid);
		disconnect_client(fd, pid, players[pid].stage != PLAYER_STAGE_NAME);

		return -1;
	}

	/* If the client sends an EOF, disconnect them. */
	if (len == 0) {
		printf("Received EOF from player %d\n", pid);
		disconnect_client(fd, pid, players[pid].stage != PLAYER_STAGE_NAME);

		return -2;
	}

	for (size_t i = 0; i < sizeof(input); i++) {
		if (input[i] == '\n' || input[i] == '\r') {
			input[i] = '\0';
			break;
		}
	}

	parsed_input = convert_string_to_json_object(input);

	/* Log the client's input. */
	printf("%d: %s\n", pid, input);

	/* If the parsed JSON object is null (indicating that the sent data was
	 * not actually JSON or malformed JSON), return. */
	if(!is_valid_json(parsed_input)) {
		printf("Player %d sent invalid JSON\n", pid);
		return 0;
	}

	switch(players[pid].stage) {
		case PLAYER_STAGE_NAME:;
			char *name;

			if (!is_type(parsed_input, "name"))
				return 0;

			arg = get_argument(parsed_input, "name");

			if (!is_valid_json(arg))
				return 0;

			name = (char *) json_object_get_string(arg);

			/* Check if the entered name is valid. */
			if (!is_valid_name(name, fd))
				return 0;

			strcpy(players[pid].name, name);

			if (state.stage == STAGE_LOBBY) {
				broadcast_json(-1, JSON_JOIN(players[pid].name));
				players[pid].stage = PLAYER_STAGE_LOBBY;
			}

			/* Greet the client. */
			send_json_data(players[pid].fd, JSON_GREETING);

			break;

		case PLAYER_STAGE_LOBBY:;
			char *message;

			if (!is_type(parsed_input, "message"))
				return 0;

			arg = get_argument(parsed_input, "message");

			if (!is_valid_json(arg))
				return 0;

			message = (char *) json_object_get_string(arg);

			int is_command = parse_command(pid, message);
			int valid = 0;

			for (size_t i = 0;  i < strlen(message); i++)
				if (!isspace(message[i]))
					valid = 1;

			struct json_object *args = json_object_new_object();

			json_object_object_add(args, "player", json_object_new_string(players[pid].name));
			json_object_object_add(args, "message", json_object_new_string(message));

			if (valid && !is_command)
				broadcast_json(fd, JSON_CHAT(message, args));

			break;

		case PLAYER_STAGE_MAIN:
			break;
	}

	return 0;
}
