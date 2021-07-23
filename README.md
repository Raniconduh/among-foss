# Among sus

A recreation of Among Us mechanics but as a multiplayer text adventure game instead of a graphical client.

```
|\----------------|--------------|----------------|--------------\
|                                                                 \
| UPPER ENGINE                        CAFETERIA       WEAPONS      \
|                 |-     --------|                |                 \
|/--------|    |--|       MEDBAY |                |                  \
          |    |                 |                |                   \------\
/---------|    |-------\         |                |----------|        |       \
|         |    |        \        |---|     |------|          |                 |
|                        \       |                |                            |
| REACTOR        SECURITY |      |  ADMIN OFFICE  |   O2           NAVIGATION  |
|                         |      |                |          |                 |
|         |    |          |      |---|     |----|-|----------|                 |
\---------|    |----------|------|              |                     |       /
          |    |                 |                                    /------/
|\--------|    |--|              |                                   /
|                 |              |              |--    --|          /
| LOWER ENGINE       ELECTRICAL       STORAGE   | COMMS  | SHIELDS /
|                                               |        |        /
|/----------------|--------------|--------------|--------|-------/
```

## Building

```shell-session
$ make
```

That's it.

## Running

```shell-session
$ ./among-sus
```

This starts the server on port 1234. then players can connect to it with any tool that does tcp sockets. For example netcat:

```shell-session
$ nc 127.0.0.1 1234
```

For an improved experience you can use `rlwrap nc` instead to don't have incoming messages mess up your input

## Joining the game

Players can only join while the game is in the pre-game lobby state. People are asked for a name and then end up in the lobby chat. One player is the admin, usually the first person to connect. That person can execute the /start command to start round. Other chat commands are described in /help

After starting the round one of the players is assigned the imposter role, all other players are assigned the crewmate role. After that the game moves to the playing state

## The playing stage

In the playing state the players get a `#` prompt. At this point people can't chat to eachother anymore and can only run commands (without / prefix)

Everyone starts in the cafeteria, everyone gets 7 tasks randomly assigned. You can check your tasks with the `check tasks` command. You complete your tasks by moving to the room the task is in and typing the full task name, this is case sensitive.

You can see the room you're in by running the `examine room` or `e` command. This will also show you where you can move to from the current room and which crewmates are in the room with you. You can move to other rooms by using the `go [roomname]` command.

The `map` command shows an ascii art diagram of the map as reference.

If you are the imposter your main mission is not to complete the tasks but to murder all the crew without anyone figuring out you're the imposter. You can use the `murder crewmate` command to kill a crewmate that's in the same room as you. If there are multiple one will be chosen at random and the other crewmates will be notified someone died in the room with them.

If somebody notices a dead body in the room they can use the `report` command to halt the playing and move everyone to the discussion stage. It is also possible to trigger the discussion stage by going to the cafeteria and using the `press emergency button` command.

## The discussion stage

If a body has been found or a meeting has been called everyone gets thrown in the discussion state. At this point everyone can see who is still alive. Then there's a chatroom which is limited to 50 messages in total, so use the chat wisely. While/after chatting it is possible to either vote to throw someone off the ship or to skip using `/vote [number]` or `/skip`. You can also use `/list` to see who has not voted yet.

After everyone has voted either the game moves directly back to the playing stage or someone gets thrown off. After the player has been ejected it will be visible if it was the imposter (and then the game has been won by the crew) or if someone innocent was ejected. Then the game moves back to the playing stage.

The crew can win by having everyone complete all the tasks or by voting out the imposter. The imposter can win by killing all the crew or by making the crew vote out all their crewmates.

# Development

There's #among-sus on libera.chat. Patches go to https://lists.sr.ht/~martijnbraam/public-inbox
