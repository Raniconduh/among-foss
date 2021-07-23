#include <arpa/inet.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#include <getopt.h>

#ifndef VERSION
#define VERSION "unknown/fork"
#endif

#define NUM_PLAYERS 10
#define NUM_SHORT 6
#define NUM_LONG 2
#define NUM_CHATS 50
#define MIN_NAME 2
#define MAX_NAME 10

#ifndef MOVEMENT_NOTIFICATIONS
#define MOVEMENT_NOTIFICATIONS 1
#endif
/*
 * Map header files
 */
#include "maps/skeld.h"
//#include "maps/polus.h" //TODO: make these functional
//#include "maps/mira.h"


// hack for auto-skeld
map = skeld_map

player_task_short = skeld_player_task_short
player_short_task_descriptions = skeld_short_task_descriptions

player_task_long = skeld_player_task_long
long_task_descriptions = skeld_long_task_descriptions

player_location = skeld_player_location
locations = skeld_locations
//i do not think the vertice enum can be called like this
descriptions = skeld_descriptions 

enum game_stage {
	STAGE_LOBBY,
	STAGE_PLAYING,
	STAGE_DISCUSS,
};

enum player_stage {
	PLAYER_STAGE_NAME,
	PLAYER_STAGE_LOBBY,
	PLAYER_STAGE_MAIN,
	PLAYER_STAGE_DISCUSS,
	PLAYER_STAGE_WAITING,
};

const char usage[] =
	"Usage: among-sus [-p <port>] [-h]\n"
	"among-sus:\tAmong Us, but it's a text adventure\n"
	"\n"
	"-p, \t\tSet port number\n"
	"-h, \t\tDisplay this message\n"
;

enum player_state {
	PLAYER_STATE_ALIVE,
	PLAYER_STATE_VENT, // TODO: implement vents
	PLAYER_STATE_DEAD,
	PLAYER_STATE_FOUND,
	PLAYER_STATE_EJECTED,
	PLAYER_STATE_KICKED,
};

struct player {
	int fd;
	enum player_stage stage;
	char name[MAX_NAME + 1];
	int is_admin;
	int is_impostor;
	enum player_location location;
	enum player_state state;
	int has_cooldown;
	int voted;
	int votes;

	enum player_task_short short_tasks[NUM_SHORT];
	int short_tasks_done[NUM_SHORT];
	enum player_task_long long_tasks[NUM_LONG];
	int long_tasks_done[NUM_LONG];
};

struct gamestate {
	enum game_stage stage;
	int has_admin;
	int players;
	int is_reactor_meltdown;
	int chats_left;
	int skips;
	unsigned int impostor_cooldown;
};

struct gamestate state;
struct player players[NUM_PLAYERS];
fd_set rfds, afds;

int random_num(int upper_bound) {
	return rand() % (upper_bound + 1);
}

int alive(struct player player) {
	return player.state == PLAYER_STATE_ALIVE || player.state == PLAYER_STATE_VENT;
}

void broadcast(char* message, int notfd){
	char buf[1024];
	int pid;

	printf("*> %s\n", message);

	snprintf(buf, sizeof(buf), "%s\n", message);

	for (pid = 0; pid < NUM_PLAYERS; pid++) {
		if (players[pid].fd == -1 || players[pid].fd == notfd
				|| players[pid].stage == PLAYER_STAGE_NAME
				|| players[pid].stage == PLAYER_STAGE_WAITING)
			continue;

		write(players[pid].fd, buf, strlen(buf));
	}
}

void broadcast_ghosts(char* message, int notfd) {
	char buf[1024];
	int pid;

	printf("*> %s\n", message);

	snprintf(buf, sizeof(buf), "%s\n", message);

	for (pid = 0; pid < NUM_PLAYERS; pid++) {
		if (players[pid].fd == -1
				|| players[pid].fd == notfd
				|| players[pid].stage == PLAYER_STAGE_NAME
				|| players[pid].stage == PLAYER_STAGE_WAITING
				|| alive(players[pid]))
			continue;

		write(players[pid].fd, buf, strlen(buf));
	}
}

void player_move(size_t pid, enum player_location location) {
	char buf[100];
	enum player_location old_location = players[pid].location;

	printf("Moving player %zu to %d\n", pid, location);
	players[pid].location = location;
	if (players[pid].has_cooldown != 0)
		--players[pid].has_cooldown;

	// body detection
	for (size_t i = 0; i < NUM_PLAYERS; i++) {
		if (players[i].location != players[pid].location || i == pid
				|| players[i].fd == -1
				|| players[i].state != PLAYER_STATE_DEAD
				|| players[i].stage != PLAYER_STAGE_WAITING)
			continue;

		snprintf(buf, sizeof(buf), "you enter the room and see the body of [%s] laying on the floor\n", players[i].name);
		write(players[pid].fd, buf, strlen(buf));
	}

	// Notify players you're moving
	if (MOVEMENT_NOTIFICATIONS) {
		for (size_t i = 0; i < NUM_PLAYERS; i++) {
			if (players[i].fd == -1 || !alive(players[i])
					|| !alive(players[pid]) || i == pid)
				continue;

			if (players[i].location == players[pid].location) {
				snprintf(buf, sizeof(buf), "[%s] just walked into the room\n", players[pid].name);
				write(players[i].fd, buf, strlen(buf));
			}
			if (players[i].location == old_location) {
				snprintf(buf, sizeof(buf), "[%s] just left the room\n", players[pid].name);
				write(players[i].fd, buf, strlen(buf));
			}
		}
	}
}

void end_game() {
	char buf[100];

	broadcast("------------------------", -1);
	broadcast("The game has ended, returning to lobby", -1);
	state.stage = STAGE_LOBBY;

	for(int i=0; i<NUM_PLAYERS;i++) {
		if (players[i].fd == -1)
			continue;

		if (players[i].stage == PLAYER_STAGE_WAITING) {
			snprintf(buf, sizeof(buf), "Game ended, sending you to lobby.\n\a");
			write(players[i].fd, buf, strlen(buf));
			snprintf(buf, sizeof(buf), "[%s] has joined the lobby.", players[i].name);
			broadcast(buf, players[i].fd);
		}

		players[i].stage = PLAYER_STAGE_LOBBY;
	}
}

int check_win_condition(void) {
	char buf[100];
	size_t nalive = 0, iid = 0, tasks = 1;

	for (size_t i = 0; i < NUM_PLAYERS; i++) {
		if (players[i].fd != -1 && players[i].is_impostor)
			iid = i;

		if (players[i].is_impostor == 1
				&& !alive(players[i])) {
			broadcast("The crew won, the impostor died", -1);
			end_game();
			return 1;
		}

		if (players[i].fd != -1 && alive(players[i])
				&& players[i].is_impostor == 0
				&& players[i].stage != PLAYER_STAGE_WAITING)
			nalive++;

		if (players[i].fd != -1 && !players[i].is_impostor
				&& players[i].stage != PLAYER_STAGE_WAITING) {
			for (size_t j = 0; j < NUM_SHORT; j++) {
				if (!players[i].short_tasks_done[j]) {
					tasks = 0;
					break;
				}
			}
			for (size_t j = 0; j < NUM_LONG; j++) {
				if (players[i].long_tasks_done[j] != 2) {
					tasks = 0;
					break;
				}
			}
		}
	}

	if (tasks == 1) {
		broadcast("The crew won, all tasks completed", -1);
		end_game();
		return 1;
	}

	if (nalive == 1) {
		broadcast("The impostor is alone with the last crewmate and murders them", -1);
		snprintf(buf, sizeof(buf), "The impostor was [%s] all along...", players[iid].name);
		broadcast(buf, -1);
		end_game();
		return 1;
	}
	return 0;
}

void task_completed(size_t pid, size_t task_id, int long_task) {
	// Mark task completed for player
	if (!long_task) {
		for (size_t i = 0; i < NUM_SHORT; i++) {
			if (players[pid].short_tasks[i] == task_id) {
				players[pid].short_tasks_done[i] = 1;
			}
		}
	} else {
		for(size_t i = 0; i < NUM_LONG; i++) {
			if (players[pid].long_tasks[i] == task_id) {
				players[pid].long_tasks_done[i]++;
			}
		}
	}

	check_win_condition();
}

void list_rooms_with_players(size_t pid) {
	int count[LOC_COUNT] = {0};
	char buf[100];

	for (size_t i = 0; i < NUM_PLAYERS; i++) {
		if (players[i].fd != -1 && alive(players[i]))
			count[players[i].location]++;
	}

	for (int i=0;i<LOC_COUNT;i++) {
		if (count[i] > 0) {
			snprintf(buf, sizeof(buf), " * There are %d players in %s\n",
					count[i], locations[i]);
			write(players[pid].fd, buf, strlen(buf));
		}
	}
}


void player_list_tasks(size_t pid) {
	char buf[100];
	int task_desc;
	int done = 1;

	for (size_t i = 0; i < TASK_SHORT_COUNT; i++) {
		for (size_t j = 0; j < NUM_SHORT; j++) {
			if(players[pid].short_tasks[j] == i) {
				const char *cm;
				if(players[pid].short_tasks_done[j]) {
					cm = "*";
				} else {
					cm = " ";
					done = 0;
				}
				snprintf(buf, sizeof(buf), " [%s] %s\n", cm,
					short_task_descriptions[i]);
				write(players[pid].fd, buf, strlen(buf));
			}
		}
	}
	for (size_t i = 0; i < TASK_LONG_COUNT; i++) {
		for (size_t j = 0; j < NUM_LONG; j++) {
			if(players[pid].long_tasks[j] == i) {
				const char *cm;
				if(players[pid].long_tasks_done[j] == 2) {
					cm = "*";
					task_desc = 0;
				} else if(players[pid].long_tasks_done[j] == 1) {
					cm = "-";
					task_desc = 1;
					done = 0;
				} else {
					cm = " ";
					task_desc = 0;
					done = 0;
				}
				snprintf(buf, sizeof(buf), " [%s] %s\n", cm,
					long_task_descriptions[i][task_desc]);
				write(players[pid].fd, buf, strlen(buf));
			}
		}
	}
	if (done) {
		snprintf(buf, sizeof(buf), "All your tasks are completed!\n# ");
	} else {
		snprintf(buf, sizeof(buf), "Complete the tasks by typing the full task name in the correct location\n# ");
	}
	write(players[pid].fd, buf, strlen(buf));
}

bool player_kill(size_t pid, size_t tid) {
	char buf[100];

	if(players[pid].location != players[tid].location
			|| players[tid].is_impostor
			|| players[tid].stage != PLAYER_STAGE_MAIN)
		return false;

	// so sad
	players[tid].state = PLAYER_STATE_DEAD;

	// less murdering, reset by movement
	players[pid].has_cooldown = state.impostor_cooldown;

	// notify player of their recent death
	snprintf(buf, sizeof(buf), "It turns out %s is the impostor, sadly the way you know is that you died.\n",
		players[pid].name);
	write(players[tid].fd, buf, strlen(buf));

	// notify bystanders
	for (size_t i = 0; i < NUM_PLAYERS; i++) {
		if (i == pid || players[i].fd == -1 || !alive(players[i])
				|| players[i].location != players[pid].location
				|| players[i].stage != PLAYER_STAGE_MAIN)
			continue;

		snprintf(buf, sizeof(buf), "someone killed [%s] while you were in the room\n",
			players[tid].name);
		write(players[i].fd, buf, strlen(buf));
	}

	check_win_condition();

	return true;
}

void start_discussion(size_t pid, size_t bid) {
	char buf[100];

	state.stage = STAGE_DISCUSS;
	state.skips = 0;

	// switch everyone to the discussion state and mark bodies found
	for(int i=0; i<NUM_PLAYERS;i++) {
		if (players[i].fd == -1
				|| players[i].stage != PLAYER_STAGE_MAIN)
			continue;

		players[i].stage = PLAYER_STAGE_DISCUSS;
		players[i].voted = 0;
		players[i].votes = 0;
	}
	broadcast("------------------------", -1);
	// Inform everyone
	if (bid == SIZE_MAX) {
		// Emergency button was pressed
		snprintf(buf, sizeof(buf), "\nAn emergency meeting was called by [%s]", players[pid].name);
	} else {
		// Body was reported
		snprintf(buf, sizeof(buf), "\nThe body of [%s] was found by [%s]", players[bid].name, players[pid].name);
		players[bid].state = PLAYER_STATE_FOUND;
	}
	broadcast(buf, -1);

	// List the state of the players
	broadcast("Players:", -1);
	for(int i=0; i<NUM_PLAYERS;i++) {
		if (players[i].fd == -1
				|| players[i].stage != PLAYER_STAGE_DISCUSS)
			continue;
		switch (players[i].state) {
		case PLAYER_STATE_ALIVE:
			snprintf(buf, sizeof(buf), "* %d [%s]", i, players[i].name);
			break;
		case PLAYER_STATE_DEAD:
			snprintf(buf, sizeof(buf), "* %d [%s] (dead)", i, players[i].name);
			break;
		case PLAYER_STATE_FOUND:
			snprintf(buf, sizeof(buf), "* %d [%s] (dead, reported)", i, players[i].name);
			break;
		case PLAYER_STATE_VENT:
		case PLAYER_STATE_EJECTED:
		case PLAYER_STATE_KICKED:
			continue;
		}
		broadcast(buf, -1);
	}

	// Inform people of the chat limit
	snprintf(buf, sizeof(buf), "Discuss, there are %d messages left", NUM_CHATS);
	state.chats_left = NUM_CHATS;
	broadcast(buf, -1);
}

void back_to_playing() {
	state.stage = STAGE_PLAYING;
	// switch everyone to the playing state
	for(int i=0; i<NUM_PLAYERS;i++) {
		if (players[i].fd == -1
				|| players[i].stage != PLAYER_STAGE_DISCUSS)
			continue;

		players[i].stage = PLAYER_STAGE_MAIN;
		players[i].location = LOC_CAFETERIA;
	}
	broadcast("-- Voting has ended, back to the ship --\n\n# ", -1);
}

void discussion(size_t pid, char *input) {
	char buf[300];
	intmax_t vote = 0, max_votes = 0, tie = 0, winner = -1, hasvalidchar = 0;
	for (size_t i = 0; i < strlen(input); i++) {
		if (!isprint(input[i]))
			input[i] = '\0';
		else if (!isspace(input[i]))
			hasvalidchar = 1;
	}

	if (input[0] == '/' && input[1] != '/') {
		if ((strncmp(input, "/vote ", 6) == 0
				|| strncmp(input, "/yeet ", 6) == 0
				|| strncmp(input, "/skip", 6) == 0)
				&& alive(players[pid]) ) {
			if (players[pid].voted) {
				snprintf(buf, sizeof(buf), "You can only vote once\n");
				write(players[pid].fd, buf, strlen(buf));
				return;
			}
			if (input[1] == 's') {
				vote = -1;
			} else {
				char *endptr = NULL;
				vote = strtol(&input[6], &endptr, 10);
				if (!endptr || endptr[0] != '\0') {
					snprintf(buf, sizeof(buf), "Invalid vote, not an integer\n");
					write(players[pid].fd, buf, strlen(buf));
					return;
				}
			}

			if (vote != -1 && (vote < -1 || vote > NUM_PLAYERS-1
					|| players[vote].fd == -1
					|| players[vote].stage != PLAYER_STAGE_DISCUSS)) {
				snprintf(buf, sizeof(buf), "Invalid vote, no such player\n");
				write(players[pid].fd, buf, strlen(buf));
				return;
			}
			if (!alive(players[vote])) {
				snprintf(buf, sizeof(buf), "Invalid vote, that person is dead\n");
				write(players[pid].fd, buf, strlen(buf));
				return;
			}
			if (vote == -1) {
				printf("[%s] voted to skip\n", players[pid].name);
			} else {
				printf("[%s] voted for %jd\n", players[pid].name, vote);
			}
			players[pid].voted = 1;
			if (vote == -1) {
				state.skips++;
			} else {
				players[vote].votes++;
			}
check_votes:
			// Check if voting is complete
			for (size_t i = 0; i < NUM_PLAYERS; i++) {
				if(players[i].fd != -1 && players[i].voted == 0
						&& alive(players[i])
						&& players[i].stage == PLAYER_STAGE_DISCUSS) {
					printf("No vote from [%s] yet\n", players[i].name);
					goto not_yet;
				}
			}

			printf("Voting complete\n");

			// Count votes
			max_votes = state.skips;
			for (size_t i = 0; i < NUM_PLAYERS; i++) {
				if(players[i].fd == -1 || players[i].stage != PLAYER_STAGE_DISCUSS)
					continue;

				if(players[i].votes > max_votes){
					max_votes = players[i].votes;
					tie = 0;
					winner = (intmax_t)i;
					continue;
				}

				if(players[i].votes == max_votes) {
					tie = 1;
				}
			}

			printf("Vote winner: %jd\n", winner);

			if (tie) {
				broadcast("The voting ended in a tie", -1);
				back_to_playing();
				return;
			} else if (winner == -1) {
				snprintf(buf, sizeof(buf), "The crew voted to skip\n");
				broadcast(buf, -1);
				back_to_playing();
				return;
			} else {
				snprintf(buf, sizeof(buf), "The crew voted to eject [%s]\n", players[winner].name);
				broadcast(buf, -1);
			}

			// dramatic pause
			for(int i=0;i<5;i++) {
				sleep(1);
				broadcast(".", -1);
			}

			players[winner].state = PLAYER_STATE_EJECTED;
			if (players[winner].is_impostor) {
				snprintf(buf, sizeof(buf), "It turns out [%s] was an impostor", players[winner].name);
				broadcast(buf, -1);
			} else {
				snprintf(buf, sizeof(buf), "Sadly, [%s] was not an impostor", players[winner].name);
				broadcast(buf, -1);

			}
			if(!check_win_condition()) {
				back_to_playing();
			}
			return;

not_yet:
			broadcast("A vote has been cast", -1);
		} else if (strncmp(input, "/help", 6) == 0) {
			snprintf(buf, sizeof(buf), "Commands: /vote [id], /skip, /list\n");
			write(players[pid].fd, buf, strlen(buf));
		} else if (strncmp(input, "/list", 6) == 0) {
			for(int i=0; i<NUM_PLAYERS;i++) {
				if (players[i].fd == -1
						|| players[i].stage != PLAYER_STAGE_DISCUSS)
					continue;
				if (alive(players[i]) && players[i].voted) {
					snprintf(buf, sizeof(buf), "* %d [%s] (voted) \n", i, players[i].name);
				} else if (alive(players[i])) {
					snprintf(buf, sizeof(buf), "* %d [%s]\n", i, players[i].name);
				} else {
					snprintf(buf, sizeof(buf), "* %d [%s] (dead)\n", i, players[i].name);
				}
				write(players[pid].fd, buf, strlen(buf));
			}

			snprintf(buf, sizeof(buf), "Commands: /vote [id], /skip, /list\n");
			write(players[pid].fd, buf, strlen(buf));
		} else if (strncmp(input, "/kick", 5) == 0) {
			if (!players[pid].is_admin) {
				snprintf(buf, sizeof(buf), "You have no kicking permission\n");
				write(players[pid].fd, buf, strlen(buf));
			}

			char *endptr = NULL;
			vote = strtol(&input[6], &endptr, 10);
			if (!endptr || endptr[0] != '\0') {
				snprintf(buf, sizeof(buf), "Invalid kick, not an integer\n");
				write(players[pid].fd, buf, strlen(buf));
				return;
			}
			snprintf(buf, sizeof(buf), "Admin kicked [%s]\n", players[vote].name);
			broadcast(buf, -1);

			close(players[vote].fd);
			FD_CLR(players[vote].fd, &afds);
			players[vote].fd = -1;
			players[vote].state = PLAYER_STATE_KICKED;
			goto check_votes;

		} else if (strncmp(input, "/me ", 4) == 0) {
			if (state.chats_left > 0 && alive(players[pid])) {
				snprintf(buf, sizeof(buf), "(%d) * [%s] %s", state.chats_left, players[pid].name, &input[4]);
				broadcast(buf, -1);
				state.chats_left--;
			} else if (alive(players[pid])) {
				snprintf(buf, sizeof(buf), "No chats left, you can only vote now\n");
				write(players[pid].fd, buf, strlen(buf));
				return;
			} else {
				snprintf(buf, sizeof(buf), "(dead) * [%s] %s", players[pid].name, &input[4]);
				broadcast_ghosts(buf, -1);
			}
		} else if (strncmp(input, "/shrug", 6) == 0) {
			if (state.chats_left > 0 && alive(players[pid])) {
				snprintf(buf, sizeof(buf), "(%d) [%s]: ¯\\_(ツ)_/¯", state.chats_left, players[pid].name);
				broadcast(buf, -1);
				state.chats_left--;
			} else if (alive(players[pid])) {
				snprintf(buf, sizeof(buf), "No chats left, you can only vote now\n");
				write(players[pid].fd, buf, strlen(buf));
				return;
			} else {
				snprintf(buf, sizeof(buf), "(dead) [%s]: ¯\\_(ツ)_/¯", players[pid].name);
				broadcast_ghosts(buf, -1);
			}
		} else {
			snprintf(buf, sizeof(buf), "Invalid command\n");
			write(players[pid].fd, buf, strlen(buf));
		}
	} else if (input[0] == '/') {
		if (state.chats_left == 0 && alive(players[pid])) {
			snprintf(buf, sizeof(buf), "No chats left, you can only vote now\n");
			write(players[pid].fd, buf, strlen(buf));
			return;
		}
		if (alive(players[pid])) {
			snprintf(buf, sizeof(buf), "(%d) [%s]: %s", state.chats_left, players[pid].name, &input[1]);
			broadcast(buf, -1);
			state.chats_left--;
		} else {
			snprintf(buf, sizeof(buf), "(dead) [%s]: %s", players[pid].name, &input[1]);
			broadcast_ghosts(buf, -1);
		}
	} else if (hasvalidchar) {
		if (state.chats_left <= 0 && alive(players[pid])) {
			snprintf(buf, sizeof(buf), "No chats left, you can only vote now\n");
			write(players[pid].fd, buf, strlen(buf));
			return;
		}

		if (alive(players[pid])) {
			snprintf(buf, sizeof(buf), "(%d) [%s]: %s", state.chats_left, players[pid].name, input);
			broadcast(buf, -1);
			state.chats_left--;
		} else {
			snprintf(buf, sizeof(buf), "(dead) [%s]: %s", players[pid].name, input);
			broadcast_ghosts(buf, -1);
		}
	}
}

void adventure(size_t pid, char *input) {
	char buf[1024];
	const char *location;
	size_t task_id;
	int task_is_long;

	if (input[0] == 'e' || strncmp(input, "ls", 3) == 0) {
		enum player_location loc = players[pid].location;
		strcpy(buf, "you can move to: ");
		assert(doors[loc][0] != LOC_COUNT);
		strncat(buf, locations[doors[loc][0]], sizeof(buf) - 1);
		for (size_t i = 1; doors[loc][i] != LOC_COUNT; i++) {
			strncat(strncat(buf, ", ", sizeof(buf) - 1),
				locations[doors[loc][i]], sizeof(buf) - 1);
		}
		strncat(buf, "\n", sizeof(buf) - 1);
		location = descriptions[loc];
		if (loc == LOC_REACTOR && state.is_reactor_meltdown) {
			location = "You are in the reactor room, there are red warning lights on and a siren is going off.\n";
		}
		write(players[pid].fd, location, strlen(location));
		write(players[pid].fd, buf, strlen(buf));
		for (size_t i = 0; i < NUM_PLAYERS; i++) {
			if (players[i].location != players[pid].location
					|| players[i].fd == -1 || i == pid
					|| players[i].stage != PLAYER_STAGE_MAIN)
				continue;

			switch (players[i].state) {
			case PLAYER_STATE_ALIVE:
				snprintf(buf, sizeof(buf),
						"you also see %s in the room with you\n",
						players[i].name);
				break;
			case PLAYER_STATE_DEAD:
				snprintf(buf, sizeof(buf),
						"you also see %s's corpse in the room with you\n",
						players[i].name);
				break;
			case PLAYER_STATE_FOUND:
				snprintf(buf, sizeof(buf),
						"you also see %s's reported corpse in the room with you\n",
						players[i].name);
				break;
			case PLAYER_STATE_VENT:
			case PLAYER_STATE_EJECTED:
			case PLAYER_STATE_KICKED:
				buf[0] = '\0';
			}
			write(players[pid].fd, buf, strlen(buf));
		}
		snprintf(buf, sizeof(buf), "# ");
	} else if (strncmp(input, "go ", 3) == 0 || strncmp(input, "cd ", 3) == 0) {
		enum player_location new;
		for (new = 0; new < LOC_COUNT; new++) {
			if (strcmp(locations[new], &input[3]) == 0) {
				break;
			}
		}
		if (new == LOC_COUNT) {
			snprintf(buf, sizeof(buf), "INVALID MOVEMENT\n# ");
		} else {
			for (size_t i = 0; doors[players[pid].location][i] != LOC_COUNT; i++) {
				if (doors[players[pid].location][i] == new) {
					player_move(pid, new);
					snprintf(buf, sizeof(buf),
						"successfully moved\n# ");
					new = LOC_COUNT;
					break;
				}
			}
			if (new != LOC_COUNT) {
				snprintf(buf, sizeof(buf), "INVALID MOVEMENT\n# ");
			}
		}
	} else if (strcmp(input, "murder crewmate") == 0) {
		if (players[pid].is_impostor == 0) {
			snprintf(buf, sizeof(buf), "you might dislike them, but you can't kill them without weapon\n# ");
		} else if (players[pid].has_cooldown) {
			snprintf(buf, sizeof(buf), "you can't kill that quickly\n# ");
		} else {
			snprintf(buf, sizeof(buf), "no one to kill here\n# ");
			for (size_t i = 0; i < NUM_PLAYERS; i++) {
				if (players[i].location != players[pid].location
						|| i == pid || players[i].fd == -1
						|| !alive(players[i])
						|| players[i].stage != PLAYER_STAGE_MAIN)
					continue;

				// TODO: kill more randomly
				if (player_kill(pid, i))
					snprintf(buf, sizeof(buf), "you draw your weapon and brutally murder %s\n# ",
						players[i].name);
				break;
			}
		}
	} else if (strcmp(input, "report") == 0) {
		for(size_t i = 0; i < NUM_PLAYERS; i++) {
			if (players[i].location != players[pid].location
					|| i == pid || players[i].fd == -1
					|| players[i].state != PLAYER_STATE_DEAD
					|| !alive(players[pid]))
				continue;

			start_discussion(pid, i);
			return;
		}

		snprintf(buf, sizeof(buf), "Nothing to report here\n# ");
	} else if (strcmp(input, "press emergency button") == 0) {
		if (players[pid].location != LOC_CAFETERIA) {
			snprintf(buf, sizeof(buf), "You can't do that here\n# ");
		} else if (!alive(players[pid])) {
			snprintf(buf, sizeof(buf), "Ghosts can't call emergencies\n# ");
		} else {
			start_discussion(pid, SIZE_MAX);
			return;
		}
	} else if (strcmp(input, "check tasks") == 0) {
		player_list_tasks(pid);
		return;
	} else if (strcmp(input, "look at monitors") == 0) {
		list_rooms_with_players(pid);
		return;
	} else if (strcmp(input, "help") == 0) {

		snprintf(buf, sizeof(buf), "Commands: help, examine room, go [room], murder crewmate, report, check tasks");
		write(players[pid].fd, buf, strlen(buf));
		switch (players[pid].location) {
			case LOC_CAFETERIA:
				snprintf(buf, sizeof(buf), "\ncommands in this room: press emergency button\n# ");
				break;
			case LOC_SECURITY:
				snprintf(buf, sizeof(buf), "\ncommands in this room: look at monitor\n# ");
				break;
			default:
				snprintf(buf, sizeof(buf), "\n# ");
				break;
		}
	} else if (strncmp(input, "map", 3) == 0) {//TODO: set map
		write(players[pid].fd, map, strlen(map));
		snprintf(buf, sizeof(buf), "# ");
	} else {
		// check if it was a task
		task_id = TASK_SHORT_COUNT + TASK_LONG_COUNT;
		task_is_long = 0;
		for (size_t i = 0; i < TASK_SHORT_COUNT; i++) {
			if(strcmp(input, short_task_descriptions[i]) == 0) {
				task_id = i;
				break;
			}
		}
		for (size_t i = 0; i < TASK_LONG_COUNT; i++) {
			for(int k=0;k<2;k++) {
				if(strcmp(input, long_task_descriptions[i][k]) == 0) {
					// Check if player has the task
					for(int l=0; l<NUM_LONG; l++) {
						if (players[pid].long_tasks[l] == i &&
								players[pid].long_tasks_done[l] == k) {
							task_id = i;
							task_is_long = 1;
						}
					}
				}
			}
		}
		if (task_id == TASK_SHORT_COUNT + TASK_LONG_COUNT) {
			snprintf(buf, sizeof(buf), "Invalid instruction\n# ");
		} else {
			// check it was in the right room
			if (strstr(input, locations[players[pid].location]) != NULL) {
				task_completed(pid, task_id, task_is_long);
				if (state.stage == STAGE_PLAYING) {
					snprintf(buf, sizeof(buf), "Completed task\n# ");
				} else {
					buf[0] = '\0';
				}
			} else {
				snprintf(buf, sizeof(buf), "You're in the wrong place for that\n# ");
			}
		}
	}
	write(players[pid].fd, buf, strlen(buf));
}

void start_game() {
	int impostornum, assigned;
	char buf[200];
	unsigned temp;

	broadcast("---------- [ Game is starting ] ----------", -1);
	broadcast("\a", -1); /* Alarm beep for y'all multitaskers */
	state.stage = STAGE_PLAYING;
	state.players = 0;
	for(int i=0; i<NUM_PLAYERS; i++) {
		if(players[i].fd != -1) {
			state.players++;
		}
	}

	// Pick an impostor
	impostornum = random_num(state.players);
	assigned = 0;
	for(int i=0; i<NUM_PLAYERS; i++) {
		if(players[i].fd == -1)
			continue;

		players[i].stage = PLAYER_STAGE_MAIN;
		players[i].location = LOC_CAFETERIA;
		players[i].state = PLAYER_STATE_ALIVE;

		// Assign NUM_SHORT random short tasks
		for(int j=0;j<NUM_SHORT;j++) {
retry:
			temp = (unsigned)random_num(TASK_SHORT_COUNT);
			for(int k=0;k<NUM_SHORT;k++) {
				if(players[i].short_tasks[k] == temp)
					goto retry;
			}
			players[i].short_tasks[j] = temp;
			players[i].short_tasks_done[j] = 0;
		}

		// Assign NUM_LONG random long tasks
		for (size_t j = 0; j < NUM_LONG; j++) {
retry2:
			temp = (unsigned)random_num(TASK_LONG_COUNT);
			for (size_t k = 0; k < NUM_LONG; k++) {
				if(players[i].long_tasks[k] == temp)
					goto retry2;
			}
			players[i].long_tasks[j] = temp;
			players[i].long_tasks_done[j] = 0;
		}

		if (assigned == impostornum) {
			players[i].is_impostor = 1;
			snprintf(buf, sizeof(buf), "You are the impostor, kill everyone without getting noticed.\n");
		} else {
			players[i].is_impostor = 0;
			snprintf(buf, sizeof(buf), "You are a crewmate, complete your tasks before everyone is killed.\n");
		}
		write(players[i].fd, buf, strlen(buf));
		assigned++;
	}


	// dramatic pause
	for(int i=0;i<5;i++) {
		sleep(1);
		broadcast(".", -1);
	}

	for(int i=0;i<NUM_PLAYERS;i++) {
		if(players[i].fd == -1)
			continue;

		if(players[i].is_impostor) {
			snprintf(buf, sizeof(buf), "You are in a spaceship, the other %d crew members think you're one of them\n# ", assigned - 1);
			write(players[i].fd, buf, strlen(buf));
		} else {
			snprintf(buf, sizeof(buf), "You are in a spaceship, one of the crew of %d people\n", assigned);
			write(players[i].fd, buf, strlen(buf));
			snprintf(buf, sizeof(buf), "The tasks have been handed out and the daily routine is starting up, but there are rumors one of your fellow crewmates isn't a crewmate at all.\n# ");
			write(players[i].fd, buf, strlen(buf));
		}
	}
}

void reassign_admin() {
	char buf[100];
	for (int i= 0; i < NUM_PLAYERS; i++) {
		if (players[i].fd == -1 || players[i].stage == PLAYER_STAGE_NAME)
			continue;
		players[i].is_admin = 1;
		state.has_admin = 1;
		snprintf(buf, sizeof(buf), " ** Admin left, new admin is %s **\n", players[i].name);
		broadcast(buf, -1);
		return;
	}
}

void set(char *buf, size_t buf_len, int fd, int pid) {
	if (strncmp(&buf[5],"kill-cooldown", 13) == 0) {
		char *nextptr = NULL;
		int value = strtol(&buf[19], &nextptr, 10);
		if (nextptr == &buf[19]) {
			const char *msg = "Error: you didn't enter any number. Leaving current value...\n";
			write(fd, msg, strlen(msg));
		} else if (value < 0) {
			const char *msg = "Error: negative numbers aren't allowed. Leaving current value...\n";
			write(fd, msg, strlen(msg));
		} else if (nextptr != NULL && nextptr[0] == '\0') {
			state.impostor_cooldown = value;
			snprintf(buf, buf_len, "%s changed impostor cooldown to %d.", players[pid].name, state.impostor_cooldown);
			broadcast(buf, -1);
		} else {
			const char *msg = "Error: invalid input. Leaving current value...\n";
			write(fd, msg, strlen(msg));
		}
	} else {
		const char *msg = "Error: you didn't write a valid property.\n";
		write(fd, msg, strlen(msg));
	}
}

void list_set(int pid) {
	char buf[100];
	snprintf(buf, sizeof(buf), " kill-cooldown = %d\n", state.impostor_cooldown);
	write(players[pid].fd, buf, strlen(buf));
}

int handle_input(int fd) {
	char buf[200] = {0};
	char buf2[300];
	ssize_t len;
	size_t pid;

	// Find player for fd
	for (pid = 0; pid < NUM_PLAYERS; pid++) {
		if (players[pid].fd == fd) {
			break;
		}
	}

	// Get the input
	len = read(fd, buf, 199);
	if (len < 0) {
		printf("Read error from player %zu\n", pid);
		players[pid].fd = -1;
		if (players[pid].stage != PLAYER_STAGE_NAME) {
			snprintf(buf, sizeof(buf), "Player [%s] disconnected.", players[pid].name);
			players[pid].name[0] = '\0';
			printf("Sending disconnection message\n");
			broadcast(buf, -1);
		}

		if (players[pid].is_admin) {
			state.has_admin = 0;
			reassign_admin();
		}

		return -1;
	}
	if (len == 0) {
		printf("Received EOF from player %zu\n", pid);
		players[pid].fd = -1;
		if (players[pid].stage != PLAYER_STAGE_NAME) {
			snprintf(buf, sizeof(buf), "Player [%s] left the game.", players[pid].name);
			players[pid].name[0] = '\0';
			printf("Sending parting message\n");
			broadcast(buf, -1);
		}

		if (players[pid].is_admin) {
			state.has_admin = 0;
			reassign_admin();
		}

		return -2;
	}

	for (size_t i = 0; i < sizeof(buf); i++) {
		if (buf[i] == '\n' || buf[i] == '\r') {
			buf[i] = '\0';
			break;
		}
	}

	printf("%zu: %s\n", pid, buf);

	switch(players[pid].stage) {
		case PLAYER_STAGE_NAME:
			// Setting the name after connection and informing the lobby
			if(strlen(buf) < MIN_NAME) {
				snprintf(buf, sizeof(buf), "Too short, pick another name\n> ");
				write(fd, buf, strlen(buf));
				return 0;
			}
			if(strlen(buf) > MAX_NAME) {
				snprintf(buf, sizeof(buf), "Too long, pick another name\n> ");
				write(fd, buf, strlen(buf));
				return 0;
			}
			for (size_t i = 0; i < NUM_PLAYERS; i++) {
				if (strcmp(players[i].name, buf) == 0) {
					snprintf(buf, sizeof(buf), "Taken, pick another name\n> ");
					write(fd, buf, strlen(buf));
					return 0;
				}
			}
			for (size_t i = 0; i < strlen(buf); i++) {
				if(!isprint(buf[i])) {
					snprintf(buf, sizeof(buf), "Invalid char, pick another name\n> ");
					write(fd, buf, strlen(buf));
					return 0;
				}
			}
			strcpy(players[pid].name, buf);

			if (state.stage == STAGE_LOBBY) {
				snprintf(buf, sizeof(buf), "[%s] has joined the lobby", players[pid].name);
				broadcast(buf, fd);
				players[pid].stage = PLAYER_STAGE_LOBBY;
			} else
				players[pid].stage = PLAYER_STAGE_WAITING;

			break;

		case PLAYER_STAGE_WAITING:
			break;

		case PLAYER_STAGE_LOBBY:
			// Chat message in the lobby
			if (buf[0] == '/') {
				if (strcmp(buf, "/start") == 0) {//TODO: set map
					if(players[pid].is_admin) {
						start_game();
					} else {
						const char *msg = "You don't have permission to /start\n";
						write(fd, msg, strlen(msg));
					}
				} else if (strcmp(buf, "/shrug") == 0) {
					snprintf(buf2, sizeof(buf2), "[%s] ¯\\_(ツ)_/¯", players[pid].name);
					broadcast(buf2, fd);
				} else if (strncmp(buf, "/me ", 4) == 0) {
					snprintf(buf2, sizeof(buf2), " * [%s] %s", players[pid].name, &buf[4]);
					broadcast(buf2, fd);
				} else if (strcmp(buf, "/help") == 0) {
					snprintf(buf, sizeof(buf), "Commands: /start, /list and more\n");
					write(fd, buf, strlen(buf));
				} else if (strcmp(buf, "/list") == 0) {
					for(int i=0;i<NUM_PLAYERS;i++){
						if (players[i].fd == -1)
							continue;
						if (players[i].stage == PLAYER_STAGE_NAME) {
							snprintf(buf, sizeof(buf), " %d: -[ setting name ]-\n", i);
						} else if (players[i].is_admin) {
							snprintf(buf, sizeof(buf), " %d: %s (admin)\n", i, players[i].name);
						} else {
							snprintf(buf, sizeof(buf), " %d: %s\n", i, players[i].name);
						}
						write(fd, buf, strlen(buf));
					}
				} else if (strncmp(buf, "/set", 4) == 0) {
					if (strlen(buf) == 4) {
						list_set(pid);
						return 0;
					}
					if (players[pid].is_admin)
						set(buf, sizeof(buf), fd, pid);
					else {
						const char *msg = "You don't have permission to /set\n";
						write(fd, msg, strlen(msg));
					}
				}
			} else {
				int hasvalidchar = 0;
				for (size_t i = 0;  i < strlen(buf); i++) {
					if (!isprint(buf[i]))
						buf[i] = '\0';
					else if (!isspace(buf[i]))
						hasvalidchar = 1;
				}

				if (hasvalidchar) {
					snprintf(buf2, sizeof(buf2), "[%s]: %s", players[pid].name, buf);
					broadcast(buf2, fd);
				}
			}
			break;
		case PLAYER_STAGE_MAIN:
			// Main game adventure loop
			adventure(pid, buf);
			break;
		case PLAYER_STAGE_DISCUSS:
			// Main discussion loop
			discussion(pid, buf);
			break;
	}

	return 0;
}

int welcome_player(int fd) {
	char buf[100];

	for (size_t i = 0; i < sizeof(players); i++) {
		if (players[i].fd > 0) {
			continue;
		}

		snprintf(buf, sizeof(buf), "among-sus server: version %s\n", VERSION);
		write(fd, buf, strlen(buf));

		if(state.stage != STAGE_LOBBY) {
			snprintf(buf, sizeof(buf), "There is a game in progress, waiting for the match to finish...\n");
			write(fd, buf, strlen(buf));
		}

		players[i].fd = fd;
		players[i].is_admin = 0;
		if (!state.has_admin) {
			state.has_admin = 1;
			players[i].is_admin = 1;
		}
		players[i].stage = PLAYER_STAGE_NAME;
		snprintf(buf, sizeof(buf), "Welcome player %zu!\n\nEnter your name:\n> ", i);
		write(fd, buf, strlen(buf));
		printf("Assigned player to spot %zu\n", i);
		return 0;
	}
	snprintf(buf, sizeof(buf), "There are no spots available, goodbye!\n");
	write(fd, buf, strlen(buf));
	close(fd);
	return -1;
}

int main(int argc, char *argv[]) {
	// Set default settings
	state.impostor_cooldown = 1;

	uint16_t port = 1234;
	char *endptr = NULL;

	int opt;
	while ((opt = getopt(argc, argv, "hp:")) != -1) {
		switch (opt) {
			case 'p':;
				errno = 0;
				long _port = strtol(optarg, &endptr, 10);
				if (*endptr != '\0' || errno != 0 ||
					_port <= 0 || _port >= 65536) {
					fprintf(stderr, "Invalid port: %s\n", optarg);
					exit(EXIT_FAILURE);
				}
				port = (uint16_t) _port;
				break;
			case 'h':
				printf("%s", usage);
				exit(EXIT_SUCCESS);
			default:
				printf("%s", usage);
				exit(EXIT_FAILURE);
		}
	}

	int listen_fd, listen6_fd, new_fd, i;
	socklen_t client_size;
	struct sockaddr_in listen_addr = {0}, client_addr = {0};
	struct sockaddr_in6 listen6_addr = {0};

	for (i = 0; i < NUM_PLAYERS; i++) {
		players[i].fd = -1;
	}
	srand((unsigned)time(NULL));

	if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("IPv4 socket");
		exit(EXIT_FAILURE);
	}
	if ((listen6_fd = socket(AF_INET6, SOCK_STREAM, 0)) == -1) {
		perror("IPv6 socket");
		exit(EXIT_FAILURE);
	}

	i = 1;
	if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR,
				&i, sizeof(i))) {
		perror("IPv4 setsockopt");
		exit(EXIT_FAILURE);
	}

	if (setsockopt(listen6_fd, SOL_SOCKET, SO_REUSEADDR,
				&i, sizeof(i))) {
		perror("IPv6 setsockopt");
		exit(EXIT_FAILURE);
	}

	listen_addr.sin_family = AF_INET;
	listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	listen_addr.sin_port = htons(port);

	listen6_addr.sin6_family = AF_INET6;
	listen6_addr.sin6_addr = in6addr_any;
	listen6_addr.sin6_port = htons(port);
	if (setsockopt(listen6_fd, IPPROTO_IPV6, IPV6_V6ONLY,
				&i, sizeof(i))) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	if (setsockopt(listen6_fd, IPPROTO_IPV6, IPV6_V6ONLY,
				&i, sizeof(i))) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	if (bind(listen_fd, (struct sockaddr *)&listen_addr, sizeof(listen_addr)) < 0) {
		perror("ipv4 bind");
		return -1;
	}
	if (bind(listen6_fd, (struct sockaddr *)&listen6_addr, sizeof(listen6_addr)) < 0) {
		perror("ipv6 bind");
		return -1;
	}

	listen(listen_fd, 5);
	listen(listen6_fd, 5);
	printf("Listening on :%d\n", port);

	state.stage = STAGE_LOBBY;

	FD_ZERO(&afds);
	FD_SET(listen_fd, &afds);
	FD_SET(listen6_fd, &afds);

	while (1) {
		rfds = afds;
		if (select(FD_SETSIZE, &rfds, NULL, NULL, NULL) < 0) {
			perror("select");
			exit(EXIT_FAILURE);
		}

		for (i = 0; i < FD_SETSIZE; ++i) {
			if (FD_ISSET (i, &rfds)) {
				if (i == listen_fd || i == listen6_fd) {
					printf("welcome client!\n");
					client_size = sizeof(client_addr);
					new_fd = accept(i, (struct sockaddr *)&client_addr, &client_size);
					if (new_fd < 0) {
						perror("accept");
						exit(EXIT_FAILURE);
					}

					printf("New connection from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
					FD_SET(new_fd, &afds);
					if(welcome_player(new_fd)<0){
						FD_CLR(new_fd, &afds);
					}
				} else {
					if(handle_input(i) < 0) {
						close(i);
						FD_CLR(i, &afds);
					}
				}
			}
		}
	}
}
