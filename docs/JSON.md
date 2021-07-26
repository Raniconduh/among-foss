# JSON Server/Client Standard

## Response and Message Types

* `info`: information
* `greeting`: a hello from the server
* `name`: responses and messages relating to user names
* `message`: commands and chat messages
* `join`: a player joined the server
* `debug`: a debug message
* `chat`: a message from another player

## Response and Message Type Arguments

### name

* `name`: the name the responses and messages are in relation to

### message

* `message`: the actual message

### join

* `player`: the name of the player who joined


### chat

* `player`: the player the message is from
* `message`: the actual message the player sent

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

A client will then send a JSON object detailing the name the player has chosen. The object may look like this:

```json
{
	"type":"name",
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

* `0`: Too Short
* `1`: Too Long
* `2`: Invalid
* `3`: Taken

A server response with the `greeting` type means the name was chosen successfully. Otherwise it is an error.

### Message

Messages may be either commands or actual chat messages. A typical message JSON object would look like this:

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
