ggg_src = files([
	'ggg_main.cc',
	'command.cc',
	'remove_entity.cc',
	'show_version.cc',
	'show_help.cc',
	'all_commands.cc',
	'ggg.cc'
])

executable(
	executable_name,
	sources: ggg_src + hierarchy_src,
	include_directories: inc,
	dependencies: [unistdx,libcrypt]
)
