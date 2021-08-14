sources = files(
	'main.c',
	'server.c',
	'client.c',
	'game.c',
	'util.c',
	'chat.c',
	'json.c',
	'task.c',
	'location.c',
	'packet.c'
)

include = include_directories('../include/')

executable(
	'among-foss',
	[sources],
	include_directories: include,
	dependencies: dependencies,
	install: true
)
