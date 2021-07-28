#include "chat.h"


/* Sanitize a string.
 * Returns the sanitized input. */
char *sanitize(char *input) {
	for (size_t i = 0;  i < strlen(input); i++)
		/* If the character is not printable, terminate the string. */
		if (!isprint(input[i]))
			input[i] = '\0';

	return input;
}

/* Check if a username is valid. */
int is_valid_name(char *name, int fd) {
	if (strlen(name) < MIN_NAME) {
		send_json_data(fd, JSON_NAME(JSON_NAME_TOO_SHORT));
		return 0;
	}

	if (strlen(name) > MAX_NAME) {
		send_json_data(fd, JSON_NAME(JSON_NAME_TOO_LONG));
		return 0;
	}

	for (size_t i = 0; i < NUM_PLAYERS; i++) {
		if (strcmp(players[i].name, name) == 0) {
			send_json_data(fd, JSON_NAME(JSON_NAME_TAKEN));
			return 0;
		}
	}

	for (size_t i = 0; i < strlen(name); i++) {
		if (!isprint(name[i])) {
			send_json_data(fd, JSON_NAME(JSON_NAME_INVALID));
			return 0;
		}
	}

	return 1;
}

/* Parse user input and do respective actions if it is a command. */
void parse_command(int pid, char *command, char *arguments[]) {
	/* If the command is nothing, return. */
	if (strlen(command) <= 0)
		return;

	switch (state.stage) {
		case STAGE_LOBBY:
			if (strcmp("start", command) == 0)
				start_game();

			break;

		case STAGE_PLAYING:
			break;

		case STAGE_DISCUSS:
			break;
	}
}
