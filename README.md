# Among Sus

A recreation of Among Us mechanics but as a multiplayer text adventure game instead of a graphical client.

## Building

```shell-session
$ meson build
$ ninja -C build
```

That's it.

## Running

```shell-session
$ ./build/src/among-sus
```

This starts the server on port 1234. Then players can connect to it with any client made for this game.
