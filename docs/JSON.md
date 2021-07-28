# JSON Server/Client Standard

## Response and Message Types

* `info`: information
* `greeting`: a hello from the server
* `name`: responses and messages relating to user names
* `message`: commands and chat messages
* `join`: a player joined the server
* `chat`: a message from another player
* `tasks`: list of tasks

## Response and Message Type Arguments

### `name`

* `name`: the name the responses and messages are in relation to

### `message`

* `message`: the actual message

### `join`

* `player`: the name of the player who joined

### `leave`

* `player`: the name of the player who left

### `chat`

* `player`: the player the message is from
* `message`: the actual message the player sent

### `location`

* `name`: the name of the location
* `doors`: an array of location names which the player are able to navigate to

### `tasks`

* `tasks`: an array of task names
->	* an array with a `name` (string), `location` (string) and `done` (boolean) object

### `set_location`

* `name`: the name of the new location

## Examples and Other Information

### Name

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

Please refer to the Game status category for the status code descriptions.

A client will then send a JSON object detailing the name the player has chosen. The object may look like this:

```json
{
	"type": "name",
	"arguments": {
		"name": "my_name"
	}
}
```

This will ask the server to set the client's name to `my_name`. In response, the server will send a JSON object like this:

```json
{
	"status": 1,
	"type": "greeting"
}
```

Name setting status codes work as such:

* `0`: Too short
* `1`: Too long
* `2`: Invalid
* `3`: Taken

A server response with the `greeting` type means the name was chosen successfully. Otherwise it is an error.

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

Commands are handled through a separate object.

### Command

A typical command JSON object would look like this:

```json
{
	"type": "command",
	"arguments": {
		"command": "start",
		"arguments": [
			"foo",
			"bar"
		]
	}
}
```

## Game
As soon as someone in the server enters starts the game and more than 2 players are connected, the game still start.

The players will get sent `game_status` with a status of `0` (Start) to indicate that the game is starting.

Then players will get sent `player_type` with their role. (`0` (Crewmate) or `1` (Impostor))

### Game Status

Game status codes work as such:

A message of this type from the server may look like this:

```json
{
	"status": 2,
	"type": "game_status"
}
```

* `0`: Start (sent when the `/start` command is executed)
* `1`: Game in Progress (sent after `info` if a game is currently in progress)
* `2`: Crew won (sent after the game finished and the crew won)
* `3`: Impostor won (sent after the game finished and the impostor won)

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
* `1`: Not in game
* `2`: Invalid
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
	"arguments": {
		"tasks": [
			{ "name": "Fix wiring", "location": "MedBay", "done": true },
			{ "name": "Swipe card", "location": "Admin", "done": false }
		]
	}
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
* `1`: Not in game
* `2`: Already completed
* `3`: Wrong location
* `4`: Doesn't exist
