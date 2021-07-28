#include "game.h"


struct game_state state;


/* Choose a random impostor. */
int choose_impostor() {
	int impostor_id = random_num(state.players);

	players[impostor_id].is_impostor = 1;
	return impostor_id;
}

void start_game() {
	/* Announce the game start. */
	broadcast_json(-1, JSON_GAME_STATUS(JSON_GAME_STATUS_START));

	/* Change the game stage. */
	state.stage = STAGE_PLAYING;

	/* Update the player count. */
	for (int i = 0; i < NUM_PLAYERS; i++)
		if (players[i].fd != -1)
			state.players++;

	choose_impostor();

	for (int i = 0; i < NUM_PLAYERS; i++) {
		if (players[i].fd == -1)
			continue;

		players[i].stage = PLAYER_STAGE_MAIN;
		players[i].location = &locations[LOC_CAFETERIA];
		players[i].state = PLAYER_STATE_ALIVE;

		/* Assign the player random tasks. */
		assign_tasks(i);
	}

	/* Tell the player about their role. */
	for (int i = 0; i < NUM_PLAYERS; i++) {
		if (players[i].fd == -1)
			continue;

		send_json_data(players[i].fd, JSON_PLAYER_TYPE(players[i].is_impostor));
	}
}
