#include "packet.h"


/* Handle a specific packet type.
 * Returns if it was actually an existing packet. */
int handle_packet(int pid, char *type, struct json_object *object) {
	struct player *player = &players[pid];
	int fd = player->fd;

	/* To start the game */
	if (strcmp("start_game", type) == 0) {

		if (player->stage == PLAYER_STAGE_LOBBY)
			start_game();

	/* To get the current location */
	} else if (strcmp("location", type) == 0) {

		if (player->stage == PLAYER_STAGE_MAIN) {
			struct json_object *args = json_object_new_object(), *doors_array = json_object_new_array();
			struct location *player_location = player->location;

			/* Create the door array. */
			for (int i = 1; i < player_location->doors_length; i++) {
				struct location *door = player_location->doors[i];
				json_object_array_add(doors_array, json_object_new_string(door->name));
			}

			json_object_object_add(args, "name", json_object_new_string(players[pid].location->name));
			json_object_object_add(args, "doors", doors_array);

			send_json_data(fd, JSON_LOCATION(1, args));
			return 1;
		}

	/* To get a list of tasks */
	} else if (strcmp("tasks", type) == 0) {

		if (player->stage == PLAYER_STAGE_MAIN) {
			struct json_object *task_list = json_object_new_array();

			for (int i = 0; i < NUM_TASKS; i++) {
				struct json_object *task_object = json_object_new_object();
				struct task *task = player->tasks[i];

				json_object_object_add(task_object, "description", json_object_new_string(task->description));
				json_object_object_add(task_object, "location", json_object_new_string(task->location->name));
				json_object_object_add(task_object, "done", json_object_new_boolean(player->tasks_done[i]));

				json_object_array_add(task_list, task_object);
			}

			send_json_data(fd, JSON_TASKS(1, task_list));
			return 1;
		}

	/* To complete a task */
	} else if (strcmp("do_task", type) == 0) {

		if (player->stage == PLAYER_STAGE_MAIN) {
			struct json_object *task_object = get_argument(object, "name");
			struct json_object *location_object = get_argument(object, "location");

			/* If not enough / invalid arguments were specified. */
			if (task_object == NULL || location_object == NULL ||
				json_object_get_type(task_object) != json_type_string ||
				json_object_get_type(location_object) != json_type_string)
				return 0;

			struct location *location = get_location_by_name((char *) json_object_get_string(location_object));

			/* If the location name could not be parsed. */
			if (location == NULL) {
				send_json_data(fd, JSON_DO_TASK(JSON_DO_TASK_DOESNT_EXIST, NULL));
				return 0;
			}

			struct task *task = get_task_by_description((char *) json_object_get_string(task_object), location);
			struct json_object *response_object = json_object_new_object();

			json_object_object_add(response_object, "name", task_object);
			json_object_object_add(response_object, "location", location_object);

			int status = do_task(pid, task);
			send_json_data(fd, JSON_DO_TASK(status, response_object));
		}

	/* To change the location */
	} else if (strcmp("set_location", type) == 0) {

		if (player->stage == PLAYER_STAGE_MAIN) {
			struct json_object *location_object = get_argument(object, "name");

			/* If a location was actually specified. */
			if (location_object == NULL) {
				send_json_data(fd, JSON_SET_LOCATION(JSON_SET_LOCATION_INVALID, NULL));
				return 0;
			}

			struct location *old_location = player->location;
			struct location *new_location = get_location_by_name((char *) json_object_get_string(location_object));

			/* If the location name could be parsed. */
			if (new_location == NULL) {
				send_json_data(fd, JSON_SET_LOCATION(JSON_SET_LOCATION_INVALID, NULL));
				return 0;
			}

			int can_move = check_doors(old_location, new_location);

			/* If the movement is not possible due to there not being a direct connection using doors. */
			if (!can_move) {
				send_json_data(fd, JSON_SET_LOCATION(JSON_SET_LOCATION_NOT_POSSIBLE, NULL));
				return 0;
			}

			/* Change the player's position. */
			int success = move_player(pid, new_location);

			if (success) {
				send_json_data(fd, JSON_SET_LOCATION(JSON_SET_LOCATION_SUCCESS, create_string_argument_pair("name", new_location->name)));
				return 1;
			} else {
				send_json_data(fd, JSON_SET_LOCATION(JSON_SET_LOCATION_ALREADY_CURRENT, NULL));
			}
		}

	/* To kill a player as the impostor */
	} else if (strcmp("kill", type) == 0) {

		if (player->stage == PLAYER_STAGE_MAIN) {
			struct json_object *target_object = get_argument(object, "name");

			/* If no player was specified, return. */
			if (target_object == NULL) {
				send_json_data(fd, JSON_KILL(JSON_KILL_INVALID_PLAYER, NULL));
				return 0;
			}

			struct player *target = get_player_by_name((char *) json_object_get_string(target_object));

			/* Kill the target. */
			int status = kill_player(player, target);

			send_json_data(fd, JSON_KILL(
							status,
							status == JSON_KILL_SUCCESS ? create_string_argument_pair("name", target->name) : NULL
						)
			);
		}

	}

	return 0;
}
