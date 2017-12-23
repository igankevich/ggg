ggg_src = files([
	'add_entity.cc',
	'align_columns.cc',
	'all_commands.cc',
	'command.cc',
	'edit_entity.cc',
	'editor.cc',
	'expire_entity.cc',
	'find_entities.cc',
	'ggg_main.cc',
	'heal.cc',
	'remove_entity.cc',
	'reset_password.cc',
	'restore_entity.cc',
	'show_duplicates.cc',
	'show_entity.cc',
	'show_expired.cc',
	'show_groups.cc',
	'show_help.cc',
	'show_members.cc',
	'show_version.cc',
	'tmpfile.cc',
])

executable(
	executable_name,
	sources: ggg_src,
	include_directories: inc,
	dependencies: [unistdx,libcrypt,ggg_core],
	install: true
)
