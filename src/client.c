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

/* Get a player using their name. */
struct player *get_player_by_name(char *name) {
	for (int pid = 0; pid < NUM_PLAYERS; pid++)
		if (strcmp(name, players[pid].name) == 0)
			return &players[pid];
	
	/* If no player could be found, return a NULL pointer. */
	return NULL;
}

int is_alive(struct player *player) {
	return player->state == PLAYER_STATE_ALIVE;
}

void notify_kill(struct player *target) {
	for (int i = 0; i < NUM_PLAYERS; i++) {
		struct player *player = &players[i];
		if (player == target || player->fd == -1 || !is_alive(player)
				|| player->location != target->location
				|| player->stage != PLAYER_STAGE_MAIN) 
			continue;

		send_json_data(player->fd, JSON_PLAYER_STATUS(JSON_PLAYER_STATUS_KILL, target->name));
	}
}

int kill_player(struct player *player, struct player *target) {
	if (target == NULL || player == NULL
			|| target->stage != PLAYER_STAGE_MAIN
			|| target->state != PLAYER_STATE_ALIVE)
		return JSON_KILL_INVALID_PLAYER;

	/* If the player is not the impostor. */
	if (!player->is_impostor || target->is_impostor)
		return JSON_KILL_NOT_IMPOSTOR;

	/* If the impostor still has a kill cooldown. */
	if (player->cooldown != 0)
		return JSON_KILL_COOLDOWN;

	/* If the target and the player are not in the same room. */
	if (player->location != target->location)
		return JSON_KILL_NOT_IN_ROOM;

	target->state = PLAYER_STATE_DEAD;
	send_json_data(target->fd, JSON_DEATH(JSON_DEATH_KILL));

	/* Notify the players in the room about the kill
	 * and check the win condition. */
	notify_kill(target);
	check_win_condition();

	/* Reset the kill cooldown. */
	player->cooldown = KILL_COOLDOWN;

	return 0;
}

/* Greet the client. */
int welcome_client(int fd) {
	send_json_data(fd, JSON_INFO);

	for (int i = 0; i < sizeof(players); i++) {
		if (players[i].fd > 0)
			continue;

		players[i].fd = fd;
		players[i].stage = PLAYER_STAGE_NAME;

		printf("Assigned player to ID %d\n", i);

		if (state.stage != STAGE_LOBBY)
			send_json_data(players[i].fd, JSON_GAME_STATUS(JSON_GAME_STATUS_IN_PROGRESS));

		return 0;
	}

	/* Tell the client that the game is full and close the file descriptor. */
	send_json_data(fd, JSON_GAME_STATUS(JSON_GAME_STATUS_FULL));
	close(fd);

	return -1;
}

/* Clean the player's info. */
void disconnect_client(struct player *player, int should_broadcast) {
	player->fd = -1;

	if (should_broadcast)
		broadcast_json(-1, JSON_PLAYER_STATUS(JSON_PLAYER_STATUS_LEAVE, player->name));

	/* This will check whether the impostor left or not enough crewmates are left. */
	check_win_condition();

	player->name[0] = '\0';
}

/* Handle client input. */
int handle_input(int fd) {
	char input[INPUT_MAX] = {0};
	char *packet_type;
	struct json_object *parsed_input, *arg;

	int pid = get_pid_by_fd(fd);
	struct player *player = &players[pid];
	int len;

	/* Get the input. */
	len = read(fd, input, INPUT_MAX - 1);

	/* If the client sends an invalid length, disconnect them. */
	if (len < 0 || len >= INPUT_MAX) {
		printf("Read error from player %d\n", pid);
		disconnect_client(player, players[pid].stage != PLAYER_STAGE_NAME);

		return -1;

	}

	/* If the client sends an EOF, disconnect them. */
	if (len == 0) {
		printf("Received EOF from player %d\n", pid);
		disconnect_client(player, players[pid].stage != PLAYER_STAGE_NAME);

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
	if (parsed_input == NULL || json_object_get_type(parsed_input) != json_type_object) {
		printf("Player %d sent invalid JSON\n", pid);
		return 0;
	}

	/* Try to handle the specified packet type.
	 * If it does not exist, continue with the execution. */
	if (handle_packet(pid, packet_type, parsed_input))
		return 0;

	/* These are special packets that I won't put in the packet handler. */
	switch(player->stage) {
		case PLAYER_STAGE_NAME:
			if (!is_type(parsed_input, "name"))
				return 0;

			arg = get_argument(parsed_input, "name");

			if (!is_valid_json(arg))
				return 0;

			char *name = (char *) json_object_get_string(arg);

			/* Check if the entered name is valid. */
			if (!is_valid_name(name, fd))
				return 0;

			strcpy(players[pid].name, name);

			if (state.stage == STAGE_LOBBY) {
				player->stage = PLAYER_STAGE_LOBBY;
				broadcast_json(fd, JSON_PLAYER_STATUS(JSON_PLAYER_STATUS_JOIN, players[pid].name));
			}

			/* Greet the client. */
			send_json_data(fd, JSON_NAME(JSON_NAME_SUCCESS));

			break;

		case PLAYER_STAGE_LOBBY:
			if (is_type(parsed_input, "message")) {
				arg = get_argument(parsed_input, "message");

				if (!is_valid_json(arg))
					return 0;

				char *message = (char *) json_object_get_string(arg);
				int valid = 0;

				for (size_t i = 0; i < strlen(message); i++)
					if (!isspace(message[i]))
						valid = 1;

				struct json_object *args = json_object_new_object();

				json_object_object_add(args, "player", json_object_new_string(player->name));
				json_object_object_add(args, "message", json_object_new_string(message));

				if (valid)
					broadcast_json(fd, JSON_CHAT(message, args));
			}

			break;

		case PLAYER_STAGE_MAIN:
			break;

		case PLAYER_STAGE_WAITING:
			break;
	}

	return 0;
}
