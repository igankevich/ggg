ggg_src = files([
	'account.cc',
	'add_entity.cc',
	'align_columns.cc',
	'all_commands.cc',
	'attach.cc',
	'backup.cc',
	'command.cc',
	'dot.cc',
	'edit_entity.cc',
	'editor.cc',
	'entity.cc',
	'entity.cc',
	'entity_type.cc',
	'expire_entity.cc',
	'expunge.cc',
	'find_entities.cc',
	'find_machines.cc',
	'form.cc',
	'ggg_main.cc',
	'group.cc',
	'guile.cc',
	'guile_traits.cc',
	'heal.cc',
	'lock_entity.cc',
	'machine.cc',
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
	'store.cc',
	'tmpfile.cc',
	'unlock_entity.cc',
])

ggg_exe = executable(
	executable_name,
	sources: ggg_src,
	include_directories: src,
	dependencies: [unistdx,ggg_crypt,ggg_core,libacl,guile],
	install: true
)

ggg_form_src = files([
	'ggg_form.cc',
]) + forms_src + password_src

executable(
	'ggg-form',
	sources: ggg_form_src,
	include_directories: src,
	dependencies: [unistdx,ggg_crypt,ggg_core,zxcvbn],
	install: true
)

install_data('types.scm')
