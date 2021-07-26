#pragma once

/* General */
#define VERSION "testing"


/* JSON packages */

/* name: "info"
 * args: { "version": current server version } */
#define JSON_INFO create_response(1, "info", create_string_argument_pair("version", VERSION))

/* name: "name"
 * status: 0 = too short, 1 = too long, 2 = invalid, 3 = taken */
#define JSON_NAME(STATUS) create_generic_response(STATUS, "name")

enum json_name_status {
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
