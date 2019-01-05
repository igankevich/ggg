ggg_src = files([
	'add_entity.cc',
	'align_columns.cc',
	'all_commands.cc',
	'attach.cc',
	'backup.cc',
	'command.cc',
	'copy.cc',
	'dot.cc',
	'edit_entity.cc',
	'editor.cc',
	'expire_entity.cc',
	'expunge.cc',
	'find_entities.cc',
	'ggg_main.cc',
	'heal.cc',
	'lock_entity.cc',
	'remove_entity.cc',
	'reset_password.cc',
	'sanitise.cc',
	'search.cc',
	'show_base.cc',
	'show_duplicates.cc',
	'show_entity.cc',
	'show_expired.cc',
	'show_groups.cc',
	'show_help.cc',
	'show_locked.cc',
	'show_members.cc',
	'show_version.cc',
	'tmpfile.cc',
	'unlock_entity.cc',
])

ggg_exe = executable(
	executable_name,
	sources: ggg_src,
	include_directories: inc,
	dependencies: [unistdx,libcrypt,ggg_core,libacl],
	install: true
)

ggg_form_src = files([
	'ggg_form.cc',
]) + forms_src

executable(
	'ggg-form',
	sources: ggg_form_src,
	include_directories: inc,
	dependencies: [unistdx,libcrypt,ggg_core,zxcvbn],
	install: true
)

