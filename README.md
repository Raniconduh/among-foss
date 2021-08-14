# Among FOSS

A recreation of Among Us mechanics but as a multiplayer text adventure game instead of a graphical client.

## Building

Both make and meson are supported build systems. The server depends only on the json-c library. To build:

```shell-session
$ make
```

or

```shell-session
$ meson build
$ ninja -C build
```

That's it.

## Running

```shell-session
$ ./build/src/among-foss
```

This starts the server on port 1234. Then players can connect to it with any client made for this game.
