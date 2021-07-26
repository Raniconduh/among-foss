#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "server.h"
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
