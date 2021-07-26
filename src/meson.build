sources = files(
	'main.c',
	'server.c',
	'client.c',
	'game.c',
	'util.c',
	'chat.c',
	'json.c'
)

include = include_directories('../include/')

executable(
	'among-sus',
	[sources],
	include_directories: include,
	dependencies: dependencies,
	install: true
)
