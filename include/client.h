#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "constant.h"
#include "chat.h"
#include "game.h"
#include "location.h"
#include "task.h"
#include "json.h"
#include "packet.h"


#define NUM_PLAYERS 10 /* Maximum amount of players */
#define MIN_NAME    2  /* Minimum name length */
#define MAX_NAME    10 /* Maximum name length */


enum player_stage {
	PLAYER_STAGE_NAME,
	PLAYER_STAGE_LOBBY,
	PLAYER_STAGE_MAIN,
	PLAYER_STAGE_WAITING
};

enum player_state {
	PLAYER_STATE_ALIVE,
	PLAYER_STATE_KICKED,
	PLAYER_STATE_DEAD
};

struct player {
	char name[MAX_NAME + 1];
	int fd;

	int is_impostor;
	int cooldown;

	struct task *tasks[NUM_TASKS];
	int tasks_done[NUM_TASKS];

	struct location *location;

	enum player_stage stage;
	enum player_state state;
};


int get_pid_by_fd(int fd);

struct player *get_player_by_name(char *name);

int is_alive(struct player *player);

int kill_player(struct player *player, struct player *target);

int welcome_client(int fd);

void disconnect_client(struct player *player, int should_broadcast);

int handle_input(int fd);
