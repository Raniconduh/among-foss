#pragma once

/* General */
#define VERSION        "testing"
#define COMMAND_PREFIX "/"
#define INPUT_MAX      200


/* JSON packets */

/* name: "info"
 * args: { "version": current server version } */
#define JSON_INFO create_response(1, "info", create_string_argument_pair("version", VERSION))

/* name: "name"
 * status: 0 = too short, 1 = too long, 2 = invalid, 3 = taken */
#define JSON_NAME(STATUS) create_generic_response(STATUS, "name")

enum json_name {
	JSON_NAME_TOO_SHORT,
	JSON_NAME_TOO_LONG,
	JSON_NAME_INVALID,
	JSON_NAME_TAKEN
};


/* name: "join"
 * args: { "player": player name (string) } */
#define JSON_JOIN(PLAYER) create_response(1, "join", create_string_argument_pair("player", PLAYER))


/* name: "leave"
 * args: { "player": player name (string) } */
#define JSON_LEAVE(PLAYER) create_response(1, "leave", create_string_argument_pair("player", PLAYER))


/* name: "kick"
 * status: 0 = game full */
#define JSON_KICK(STATUS) create_generic_response(STATUS, "kick")

enum json_kick_status {
	JSON_KICK_GAME_FULL
};


/* name: "greeting" */
#define JSON_GREETING create_generic_response(1, "greeting")


/* name: "chat"
 * status: 1 
 * args: { "message": chat message (string) } */
#define JSON_CHAT(MESSAGE, ARGUMENTS) create_response(1, "chat", ARGUMENTS)


/* name: "game_status"
 * status: 0 = start, 1 = in progress, 2 = crew won, 3 = impostor won */
#define JSON_GAME_STATUS(STATUS) create_generic_response(STATUS, "game_status")

enum json_game_status {
	JSON_GAME_STATUS_START,
	JSON_GAME_STATUS_IN_PROGRESS,
	JSON_GAME_STATUS_CREW_WON,
	JSON_GAME_STATUS_IMPOSTOR_WON
};


/* name: "player_type"
 * status: 0 = crewmate, 1 = impostor, 2 = ghost (only gets sent after being killed by the impostor */
#define JSON_PLAYER_TYPE(STATUS) create_generic_response(STATUS, "player_type")

enum json_player_type {
	JSON_PLAYER_TYPE_CREWMATE,
	JSON_PLAYER_TYPE_IMPOSTOR,
	JSON_PLAYER_TYPE_GHOST
};


/* name: "location"
 * status: 0 = failed, 1 = succeeded
 * args: { "name": location name (string), "doors": list of locations the player can move to (string array) } */
#define JSON_LOCATION(STATUS, ARGS) create_response(STATUS, "location", ARGS)


/* name: "tasks"
 * args: { "tasks": array of task objects (refer to the documentation for more info) } */
#define JSON_TASKS(STATUS, ARGS) create_response(STATUS, "tasks", ARGS)


/* name: "set_location"
 * status: 0 = success, 1 = invalid location, 2 = already current
 * args: { "name": name of the new location } */
#define JSON_SET_LOCATION(STATUS, ARGS) create_response(STATUS, "set_location", ARGS)

enum json_set_location {
	JSON_SET_LOCATION_SUCCESS,
	JSON_SET_LOCATION_NOT_IN_GAME,
	JSON_SET_LOCATION_INVALID,
	JSON_SET_LOCATION_ALREADY_CURRENT
};

/* name: "do_task"
 * status: 0 = success, 1 = already completed, 2 = wrong location, 3 = doesn't exist */
#define JSON_DO_TASK(STATUS) create_generic_response(STATUS, "do_task")

enum json_do_task {
	JSON_DO_TASK_SUCCESS,
	JSON_DO_TASK_NOT_IN_GAME,
	JSON_DO_TASK_ALREADY_COMPLETED,
	JSON_DO_TASK_WRONG_LOCATION,
	JSON_DO_TASK_DOESNT_EXIST
};
