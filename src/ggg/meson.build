ggg_src = files([
	'ggg_main.cc',
	'command.cc',
	'remove_entity.cc',
	'expire_entity.cc',
	'restore_entity.cc',
	'edit_entity.cc',
	'add_entity.cc',
	'show_version.cc',
	'show_help.cc',
	'all_commands.cc',
	'editor.cc',
	'align_columns.cc',
	'tmpfile.cc',
])

executable(
	executable_name,
	sources: ggg_src + hierarchy_src,
	include_directories: inc,
	dependencies: [unistdx,libcrypt],
	install: true
)
