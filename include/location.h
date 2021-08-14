#pragma once

#include <stdio.h>
#include <stdlib.h>


enum location_id {
	LOC_CAFETERIA,
	LOC_REACTOR,
	LOC_UPPER_ENGINE,
	LOC_LOWER_ENGINE,
	LOC_SECURITY,
	LOC_MEDBAY,
	LOC_ELECTRICAL,
	LOC_STORAGE,
	LOC_ADMIN,
	LOC_COMMUNICATIONS,
	LOC_O2,
	LOC_WEAPONS,
	LOC_SHIELDS,
	LOC_NAVIGATION,
	LOC_COUNT
};

struct location {
	char *name;

	struct location *doors[LOC_COUNT];
	int doors_length;
};


extern const char map[];

extern struct location locations[];

struct location *get_location_by_name(char *name);

void notify_movement(int pid, struct location *old_location);

void notify_bodies(int pid);

int check_doors(struct location *current, struct location *new);

int move_player(int pid, struct location *new_location);
