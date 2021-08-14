# JSON Server/Client Standard

## Response and Message Types

* `info`: information
* `name`: responses and messages relating to user names
* `message`: chat messages
* `player_status`: status of a player
* `chat`: a message from another player
* `tasks`: list of tasks

## Response and Message Type Arguments

### `info`

* `version`: server version

### `name`

* `name`: the name the responses and messages are in relation to

### `message`

* `message`: the actual message

### `player_status`

* `player`: the name of the player who joined

### `chat`

* `player`: the player the message is from
* `message`: the actual message the player sent

### `location`

* `name`: the name of the location
* `doors`: an array of location names which the player are able to navigate to

### `tasks`

* `arguments` is an array, each object has a `name` (string), `location` (string) and `done` (boolean) object

### `set_location`

* `name`: the name of the new location

### `kill`

* `name`: name of the player that got killed

### `vote`

* `name`: name of the player that got voted

# Examples and Other Information

# Connecting

### Info

On connection, the server will send out a JSON object with the server version like so:

```json
{
	"type": "info",
	"arguments": {
		"version": "1.0"
	}
}
```

The server may send an object informing the client that a game is currently in progress. It will look like this:

```json
{
	"status": 1,
	"type": "game_status"
}
```

If that is not the case, it may send an object indicating that the game is full and close the file descriptor:

```json
{
	"status": 2,
	"type": "game_status"
}
```

Please refer to the `game_status` section for the status code descriptions.

A client will then need to send a JSON object detailing the name it has chosen. The object may look like this:

```json
{
	"type": "name",
	"arguments": {
		"name": "my_name"
	}
}
```

All printable characters (validated using `isprint()`) are allowed.

This will ask the server to set the client's name to `my_name`. In response, the server will send a JSON object like this:

```json
{
	"status": 1,
	"type": "name"
}
```

Name setting status codes work as such:

* `0`: Success
* `1`: Too short
* `2`: Too long
* `3`: Invalid
* `4`: Taken

A server response with the `name` type and status code of `0` means the name was chosen successfully. Otherwise it is an error.

## Game

As soon as someone in the server starts the game and more than 2 players are connected, the game still start.

The players will get sent `game_status` with a status of `0` (Start) to indicate that the game is starting.

Then players will get sent `player_type` with their role. (`0` (Crewmate) or `1` (Impostor))

### Player Status

Whenever a player leaves/joins/gets kicked/etc. the server will send a `player_status` object which may look like this:

```json
{
	"status": 1,
	"type": "player_status",
	"arguments": {
		"player": "someone"
	}
}
```

The status codes mean the following:

* `0`: Leave
* `1`: Join
* `2`: Room Leave
* `3`: Room Enter
* `4`: Body (gets sent when you enter a room with a body)
* `5`: Kill (was killed by the impostor in the current room)
* `5`: Vote (voted out by discussion)

### Message

A typical message JSON object would look like this:

```json
{
	"type": "message",
	"arguments": {
		"message": "Hello, World!"
	}
}
```

All printable characters (validated using `isprint()`) are allowed.

Other players in the server will get an object like this:

```json
{
	"status": 1,
	"type": "chat",
	"arguments": {
		"player": "some guy",
		"message": "Hello, World!"
	}
}
```

### Game Status

Game status codes work as such:

A message of this type from the server may look like this:

```json
{
	"status": 3,
	"type": "game_status"
}
```

* `0`: Start (sent when the game started)
* `1`: In Progress (sent after `info` if a game is currently in progress)
* `2`: Full (sent after `info` if the game is full)
* `3`: Crew won (sent after the game finished and the crew won)
* `4`: Impostor won (sent after the game finished and the impostor won)

### Player Type

`player_type` will be sent after the game started. (after `game_status` with a `0` status code) 

It indicates whether the player has been chosen as an impostor or crewmate.

A message of this type from the server may look like this:

```json
{
	"status": 1,
	"type": "player_type"
}
```

Player type status codes work as such:

* `0`: Crewmate
* `1`: Impostor
* `2`: Ghost (only gets sent after being killed by the impostor)

### Start Game

If the current game state is not `lobby` or `discussion`, sending the `start_game` event as a client will start the game.

A request from the client should look like this:

```json
{
	"type": "start_game"
}
```

### Location

Clients are able to query their location at any time, except when they are currently at the `lobby`, `waiting` or `name` stage.

The status code will be `1` if the client has the ability to get its current location and `0` if the client is not able to get its current location.

A request from the client should look like this:

```json
{
	"type": "location"
}
```

A successful response from the server may look like this:

```json
{
	"status": "1",
	"type": "location",
	"arguments": {
		"name": "MedBay",
		"doors": [
			"Admin",
			"Cafeteria",
			"Reactor"
		]
	}
}
```

A failed response from the server will look like this:

```json
{
	"status": 0,
	"type": "location"
}
```

### Set Location

Clients are able to set their location at any time, except when they are currently at the `lobby`, `waiting` or `name` stage.

The status code will be `1` if the move was successful and `0` if the move was not possible (already at the specified location).

A request from the client should look like this:

```json
{
	"type": "set_location",
	"arguments": {
		"name": "MedBay"
	}
}
```

A successful response from the server may look like this:

```json
{
	"status": 0,
	"type": "set_location",
	"arguments": {
		"name": "MedBay"
	}
}
```

A failed response from the server may look like this:

```json
{
	"status": 2,
	"type": "set_location"
}
```

The status codes mean the following:

* `0`: Success
* `1`: Invalid
* `2`: Not possible
* `3`: Already current

### Tasks

Clients are able to query their tasks at any time, except when they are currently at the `lobby`, `waiting` or `name` stage.

The status code will be `1` if you have the ability to get the task list and `0` if you are not able to get the task list.

A request from the client should look like this:

```json
{
	"type": "tasks"
}
```

A successful response from the server may look like this:

```json
{
	"status": 1,
	"type": "tasks",
	"arguments": [
		{ "name": "Fix wiring", "location": "MedBay", "done": true },
		{ "name": "Swipe card", "location": "Admin", "done": false }
	]
}
```

A failed response from the server will look like this:

```json
{
	"status": 0,
	"type": "tasks"
}
```

### Do Task

To complete a task as a client, send a request looking like this:

```json
{
	"type": "do_task",
	"arguments": {
		"name": "Fix wiring",
		"location": "MedBay"
	}
}
```

The server may send a response like this:

```json
{
	"status": 1,
	"type": "do_task"
}
```

The status codes mean the following:

* `0`: Success
* `1`: Already completed
* `2`: Wrong location
* `3`: Doesn't exist

### Kill

Only possible during the game. The player also needs to be the impostor.

As a client, send a request looking like this:

```json
{
	"type": "kill",
	"arguments": {
		"name": "target"
	}
}
```

The server may send a response like this:

```json
{
	"status": 1,
	"type": "kill",
	"arguments": {
		"name": "target"
	}
}
```

The `target` argument is only given if it was successful.

The status codes mean the following:

* `0`: Success
* `1`: Not in room
* `2`: Cooldown
* `3`: Invalid Player (player doesn't exist, not in game)
* `4`: Not Impostor (the sender is not the impostor)

All the players in the room where the kill happened will get an object looking like this:

```json
{
	"status": 3,
	"type": "player_status",
	"arguments": {
		"player": "someone"
	}
}
```

Refer to the `player_status` section for more information about the status codes.

### Death

Will only get sent to a client. The JSON object may look like this:

```json
{
	"status": 0,
	"type": "death"
}
```

The status codes mean the following:

* `0`: Kill (killed by the impostor)
* `1`: Vote (voted out by discussion)
