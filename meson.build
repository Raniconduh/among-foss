project('among-foss', 'c')

cc = meson.get_compiler('c')
json_c = cc.find_library('json-c', required: true)

dependencies = [
	json_c
]

subdir('src')
