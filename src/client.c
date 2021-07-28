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

		if (state.stage != STAGE_LOBBY)
			broadcast_json(-1, JSON_GAME_STATUS(JSON_GAME_STATUS_IN_PROGRESS));

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

	/* TODO: Reset the game if no more players are in the game
	 * TODO: Check the win condition */

	players[pid].name[0] = '\0';
}

/* Handle a specific packet type.
 * Returns if it was actually an existing packet. */
int handle_packet(int pid, char *type, struct json_object *object) {
	struct player *player = &players[pid];
	int fd = player->fd;

	/* To get the current location */
	if (strcmp("location", type) == 0) {

		if (player->stage == PLAYER_STAGE_MAIN) {
			struct json_object *args = json_object_new_object(), *doors_array = json_object_new_array();

			struct location *player_location = player->location;
			int doors_size = sizeof *player_location->doors / sizeof(struct location *);
			
			for (int i = 0; i < doors_size; i++) {
				struct location *door = player_location->doors[i];	
				json_object_array_add(doors_array, json_object_new_string(door->name));
			}

			json_object_object_add(args, "name", json_object_new_string(players[pid].location->name));
			json_object_object_add(args, "doors", doors_array);

			send_json_data(fd, JSON_LOCATION(1, args));
			return 1;
		} else
			send_json_data(fd, JSON_LOCATION(0, NULL));

	/* To get a list of tasks */
	} else if (strcmp("tasks", type) == 0) {

		if (player->stage == PLAYER_STAGE_MAIN) {
			struct json_object *task_list = json_object_new_array();

			for (int i = 0; i < NUM_TASKS; i++) {
				struct json_object *task_object = json_object_new_object();
				struct task *task = player->tasks[i];

				json_object_object_add(task_object, "description", json_object_new_string(task->description));
				json_object_object_add(task_object, "location", json_object_new_string(task->location->name));
				json_object_object_add(task_object, "done", json_object_new_boolean(player->tasks_done[i]));

				json_object_array_add(task_list, task_object);
			}

			send_json_data(fd, JSON_TASKS(1, task_list));
			return 1;
		} else
			send_json_data(fd, JSON_TASKS(0, NULL));

	/* To complete a task */
	} else if (strcmp("do_task", type) == 0) {

		/* TODO: Implement */

	/* To change the location */
	} else if (strcmp("set_location", type) == 0) {

		if (player->stage == PLAYER_STAGE_MAIN) {
			struct location *location = player->location;

			if (location != NULL) {
				struct json_object *location_object = get_argument(object, "name");
				struct location *new_location;
				int success;
				
				if (location_object == NULL) {
					send_json_data(fd, JSON_SET_LOCATION(JSON_SET_LOCATION_INVALID, NULL));
					return 0;
				}

				new_location = parse_location_name((char *) json_object_get_string(location_object));

				if (new_location == NULL) {
					send_json_data(fd, JSON_SET_LOCATION(JSON_SET_LOCATION_INVALID, NULL));
					return 0;
				}

				success = move_player(pid, new_location);

				if (success) {
					send_json_data(fd, JSON_SET_LOCATION(JSON_SET_LOCATION_SUCCESS, create_string_argument_pair("name", new_location->name)));
					return 1;
				} else
					send_json_data(fd, JSON_SET_LOCATION(JSON_SET_LOCATION_ALREADY_CURRENT, NULL));
			}
		} else
			send_json_data(fd, JSON_SET_LOCATION(JSON_SET_LOCATION_NOT_IN_GAME, NULL));

	}

	return 0;
}

/* Handle client input. */
int handle_input(int fd) {
	char input[INPUT_MAX] = {0};
	char *packet_type;
	struct json_object *parsed_input, *arg;

	int pid = get_pid_by_fd(fd);
	int len;

	/* Get the input. */
	len = read(fd, input, INPUT_MAX - 1);

	/* If the client sends an invalid length, disconnect them. */
	if (len < 0 || len > INPUT_MAX) {
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
	packet_type = get_type(parsed_input);

	/* Log the client's input. */
	printf("%d: %s\n", pid, input);

	/* If the parsed JSON object is null (indicating that the sent data was
	 * not actually JSON or malformed JSON), return. */
	if (!is_valid_json(parsed_input)) {
		printf("Player %d sent invalid JSON\n", pid);
		return 0;
	}

	/* Try to handle the specified packet type.
	 * If it does not exist, continue with the execution. */
	if (handle_packet(pid, packet_type, parsed_input))
		return 0;

	/* These are special packets that I won't put in the packet handler. */
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
				players[pid].stage = PLAYER_STAGE_LOBBY;
				broadcast_json(fd, JSON_JOIN(players[pid].name));
			}

			/* Greet the client. */
			send_json_data(players[pid].fd, JSON_GREETING);

			break;

		case PLAYER_STAGE_LOBBY:
			if (is_type(parsed_input, "message")) {
				char *message;
				arg = get_argument(parsed_input, "message");

				if (!is_valid_json(arg))
					return 0;

				message = (char *) json_object_get_string(arg);

				int valid = 0;

				for (size_t i = 0; i < strlen(message); i++)
					if (!isspace(message[i]))
						valid = 1;

				struct json_object *args = json_object_new_object();

				json_object_object_add(args, "player", json_object_new_string(players[pid].name));
				json_object_object_add(args, "message", json_object_new_string(message));

				if (valid)
					broadcast_json(fd, JSON_CHAT(message, args));
			} else if (is_type(parsed_input, "command")) {
				printf("b\n");

				struct json_object *command = get_argument(parsed_input, "command");
				struct json_object *arguments = get_argument(parsed_input, "arguments");

				/* If no "command" or "arguments" arguments were given, return. */
				if (command == NULL || arguments == NULL)
					return 0;

				/* If the arguments are not the correct type, return. */
				if (json_object_get_type(command) != json_type_string
						|| json_object_get_type(arguments) != json_type_array)
					return 0;

				char *string_command = (char *) json_object_get_string(command);
				char *string_arguments[10];

				for (int i = 0; i < json_object_array_length(arguments); i++) {
					struct json_object *argument = json_object_array_get_idx(arguments, i);

					if (argument == NULL
							|| json_object_get_type(argument) != json_type_string)
						continue;

					string_arguments[i] = (char *) json_object_get_string(argument);
				}

				parse_command(pid, string_command, string_arguments);
			}

			break;

		case PLAYER_STAGE_MAIN:
			break;

		case PLAYER_STAGE_WAITING:
			break;
	}

	return 0;
}
