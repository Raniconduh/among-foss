#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "constant.h"
#include "chat.h"
#include "game.h"
#include "json.h"


#define NUM_PLAYERS 10 /* Maximum amount of players */
#define MIN_NAME    2  /* Minimum name length */
#define MAX_NAME    10 /* Maximum name length */

enum player_stage {
	PLAYER_STAGE_NAME,
	PLAYER_STAGE_LOBBY,
	PLAYER_STAGE_MAIN
};

enum player_state {
	PLAYER_STATE_ALIVE,
	PLAYER_STATE_KICKED
};

struct player {
	char name[MAX_NAME + 1];
	int fd;

	int is_impostor;
	int has_cooldown;

	//enum player_task tasks[NUM_TASKS];
	//int short_tasks_done[NUM_TASKS];

	//enum player_location location;

	enum player_stage stage;
	enum player_state state;
};


int get_pid_by_fd(int fd);

int welcome_client(int fd);

void disconnect_client(int fd, int pid, int should_broadcast);

int handle_input(int fd);
