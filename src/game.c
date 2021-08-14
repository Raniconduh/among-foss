#include "game.h"


struct game_state state;


int check_win_condition() {
	int alive_players = 0, tasks_done = 1;
	struct player *impostor;

	/* If the game isn't running. */
	if (state.stage != STAGE_PLAYING)
		return -1;

	for (int i = 0; i < NUM_PLAYERS; i++) {
		struct player *player = &players[i];

		/* If the player disconnected, continue with the next player. */
		if (player->fd == -1 && player->is_impostor)
			end_game(JSON_GAME_STATUS_CREW_WON);
		else if (player->fd == -1)
			continue;
		
		/* If the player is the impostor. */
		if (player->is_impostor)
			impostor = player;

		/* If the player is the impostor and not alive anymore. */
		if (player->is_impostor && !is_alive(player)) {
			end_game(JSON_GAME_STATUS_CREW_WON);
			return 1;
		}

		/* If the player is just a crewmate and not in the "waiting" stage, increment the alive player counter. */
		if (!player->is_impostor && is_alive(player) && player->stage != PLAYER_STAGE_WAITING)
			alive_players++;

		/* Check if all tasks are done yet. */
		if (!player->is_impostor && player->stage != PLAYER_STAGE_WAITING) {
			for (int j = 0; j < NUM_TASKS; j++) {
				if (!player->tasks_done[i]) {
					tasks_done = 0;
					break;
				}
			}
		}
	}

	/* If all tasks were completed. */
	if (tasks_done) {
		end_game(JSON_GAME_STATUS_CREW_WON);
		return 1;
	}

	/* If only one player is left. */
	if (alive_players == 1) {
		end_game(JSON_GAME_STATUS_IMPOSTOR_WON);
		return 1;
	}

	return 0;
}

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

	/* Choose a random impostor. */
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

void end_game(enum json_game_status winner) {

	printf("ok\n");
	/* Announcing the game end. */
	broadcast_json(-1, JSON_GAME_STATUS(winner));

	/* Change the game stage back to the lobby. */
	state.stage = STAGE_LOBBY;

	for (int i = 0; i < NUM_PLAYERS; i++) {
		if (players[i].fd == -1)
			continue;

		/* Change the player's stage back to the lobby. */
		players[i].stage = PLAYER_STAGE_LOBBY;
	}
}
