#include "location.h"
#include "server.h"


const char map[] =
	"|\\----------------|--------------|----------------|--------------\\\n"
	"|                                                                 \\\n"
	"| UPPER ENGINE                        CAFETERIA       WEAPONS      \\\n"
	"|                 |-     --------|                |                 \\\n"
	"|/--------|    |--|       MEDBAY |                |                  \\\n"
	"          |    |                 |                |                   \\------\\\n"
	"/---------|    |-------\\         |                |----------|        |       \\\n"
	"|         |    |        \\        |---|     |------|          |                 |\n"
	"|                        \\       |                |                            |\n"
	"| REACTOR        SECURITY |      |  ADMIN OFFICE  |   O2           NAVIGATION  |\n"
	"|                         |      |                |          |                 |\n"
	"|         |    |          |      |---|     |----|-|----------|                 |\n"
	"\\---------|    |----------|------|              |                     |       /\n"
	"          |    |                 |                                    /------/\n"
	"|\\--------|    |--|              |                                   /\n"
	"|                 |              |              |--    --|          /\n"
	"| LOWER ENGINE       ELECTRICAL       STORAGE   | COMMS  | SHIELDS /\n"
	"|                                               |        |        /\n"
	"|/----------------|--------------|--------------|--------|-------/\n"
;

struct location locations[] = {
	/*                  Name         Description  Doors */
	[LOC_CAFETERIA] = { "Cafeteria", "yes",       { &locations[LOC_ADMIN]  }    },
	[LOC_STORAGE] =   { "Storage",   "no",        { &locations[LOC_ADMIN] }     },
	[LOC_ADMIN]   =   { "Admin",     "maybe",     { &locations[LOC_CAFETERIA] } }
};


/* Convert a location string to a location enum. */
struct location *parse_location_name(char *name) {
	for (int i = 0; i < LOC_COUNT; i++) {
		struct location *location = &locations[i];

		/* If the location doesn't actually exist, continue with the next object. */
		if (location == NULL || location->name == NULL)
			continue;

		/* If the name of the argument and found object matches, return it. */
		if (strcmp(name, location->name) == 0) {
			return location;
		}
	}

	return NULL;
}

void notify_movement(int pid, struct location *old_location, struct location *new_location) {
	/* TODO: Implement */
}

/* Move the player to a new location.
 * Returns whether the move was successful. */
int move_player(int pid, struct location *new_location) {
	struct player *player = &players[pid];

	struct location *old_location = player->location;

	/* TODO: Decrease impostor cooldown */

	if (new_location != old_location) {
		player->location = new_location;
		notify_movement(pid, old_location, new_location);

		return 1;
	}

	return 0;
}
