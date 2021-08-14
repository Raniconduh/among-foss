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
	[LOC_CAFETERIA] =      { "Cafeteria",      { &locations[LOC_MEDBAY], &locations[LOC_ADMIN], &locations[LOC_WEAPONS] },                                     3 },
	[LOC_REACTOR] =        { "Reactor",        { &locations[LOC_UPPER_ENGINE], &locations[LOC_SECURITY], &locations[LOC_LOWER_ENGINE] },                       3 },
	[LOC_UPPER_ENGINE] =   { "Upper Engine",   { &locations[LOC_REACTOR], &locations[LOC_SECURITY], &locations[LOC_MEDBAY] },                                  3 },
	[LOC_LOWER_ENGINE] =   { "Lower Engine",   { &locations[LOC_REACTOR], &locations[LOC_SECURITY], &locations[LOC_ELECTRICAL] },                              3 },
	[LOC_SECURITY] =       { "Security",       { &locations[LOC_UPPER_ENGINE], &locations[LOC_REACTOR], &locations[LOC_LOWER_ENGINE] },                        3 },
	[LOC_MEDBAY] =         { "MedBay",         { &locations[LOC_UPPER_ENGINE], &locations[LOC_CAFETERIA] },                                                    2 },
	[LOC_ELECTRICAL] =     { "Electrical",     { &locations[LOC_LOWER_ENGINE], &locations[LOC_STORAGE] },                                                      2 },
	[LOC_STORAGE] =        { "Storage",        { &locations[LOC_ELECTRICAL], &locations[LOC_ADMIN], &locations[LOC_COMMUNICATIONS], &locations[LOC_SHIELDS] }, 4 },
	[LOC_ADMIN]  =         { "Admin",          { &locations[LOC_CAFETERIA], &locations[LOC_STORAGE] },                                                         2 },
	[LOC_COMMUNICATIONS] = { "Communications", { &locations[LOC_STORAGE], &locations[LOC_SHIELDS] },                                                           2 },
	[LOC_O2] =             { "O2",             { &locations[LOC_SHIELDS], &locations[LOC_WEAPONS], &locations[LOC_NAVIGATION] },                               3 },
	[LOC_WEAPONS] =        { "Weapons",        { &locations[LOC_CAFETERIA], &locations[LOC_O2], &locations[LOC_NAVIGATION] },                                  3 },
	[LOC_SHIELDS] =        { "Shields",        { &locations[LOC_STORAGE], &locations[LOC_COMMUNICATIONS], &locations[LOC_O2], &locations[LOC_NAVIGATION] },    4 },
	[LOC_NAVIGATION] =     { "Navigation",     { &locations[LOC_WEAPONS], &locations[LOC_O2], &locations[LOC_SHIELDS] },                                       3 }
};


/* Convert a location string to a location struct. */
struct location *get_location_by_name(char *name) {
	for (int i = 0; i < LOC_COUNT; i++) {
		struct location *location = &locations[i];

		/* If the location doesn't actually exist, continue with the next object. */
		if (location == NULL || location->name == NULL)
			continue;

		/* If the name of the argument and found object matches, return it. */
		if (strcmp(name, location->name) == 0)
			return location;
	}

	return NULL;
}

/* Notify all other players in the room about movement. */
void notify_movement(int pid, struct location *old_location) {
	struct player *player = &players[pid];

	for (int i = 0; i < NUM_PLAYERS; i++) {
		struct player *plr = &players[i];

		if (plr == NULL || plr->fd == -1 || !is_alive(plr)
				|| !is_alive(player) || player == plr)
			continue;

		/* If the player entered the room. */
		if (plr->location == player->location)
			send_json_data(plr->fd, JSON_PLAYER_STATUS(JSON_PLAYER_STATUS_ROOM_ENTER, player->name));
		
		/* If the player left the room. */
		if (plr->location == old_location)
			send_json_data(plr->fd, JSON_PLAYER_STATUS(JSON_PLAYER_STATUS_ROOM_LEAVE, player->name));
	}
}

/* Notify a player about bodies in the room. */
void notify_bodies(int pid) {
	struct player *player = &players[pid];

	for (int i = 0; i < NUM_PLAYERS; i++) {
		struct player *plr = &players[i];

		if (plr == NULL || plr->fd == -1 || is_alive(plr)
				|| plr->is_impostor || plr == player)
			continue;

		/* Notify the player. */
		send_json_data(player->fd, JSON_PLAYER_STATUS(JSON_PLAYER_STATUS_BODY, plr->name));
	}
}

/* Check if it it possible to go from current -> new using the doors. */
int check_doors(struct location *current, struct location *new) {
	struct location *door;

	for (int i = 0; i < current->doors_length; i++) {
		if (current->doors[i] == new) {
			door = current->doors[i];
		}
	}
	
	return door != NULL;
}

/* Move the player to a new location.
 * Returns whether the move was successful. */
int move_player(int pid, struct location *new_location) {
	struct player *player = &players[pid];
	struct location *old_location = player->location;

	if (new_location != old_location) {
		player->location = new_location;
		notify_movement(pid, old_location);

		/* Decrease the player's cooldown. */
		if (player->cooldown != 0)
			--player->cooldown;

		/* Notify about bodies. */
		notify_bodies(pid);

		return 1;
	}

	return 0;
}
