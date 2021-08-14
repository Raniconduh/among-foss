#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "server.h"
#include "task.h"
#include "client.h"


enum game_stage {
	STAGE_LOBBY,
	STAGE_PLAYING,
	STAGE_DISCUSS
};


struct game_state {
	enum game_stage stage;

	int players;
	int skips;

	unsigned int impostor_cooldown;
};


extern struct game_state state;


int check_win_condition();

int choose_impostor();

void start_game();

void end_game(enum json_game_status winner);
