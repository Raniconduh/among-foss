#pragma once

#include <stdio.h>
#include <stdlib.h>


enum location_id {
	LOC_CAFETERIA,
	LOC_MEDBAY,
	LOC_STORAGE,
	LOC_ADMIN,
	LOC_COUNT
};

struct location {
	char *name;
	char *description;
	struct location *doors[LOC_COUNT - 1];
};


extern const char map[];

extern struct location locations[];


struct location *parse_location_name(char *name);

void notify_movement(int pid, struct location *old_location, struct location *new_location);

int move_player(int pid, struct location *new_location);
