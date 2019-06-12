ggg_src = files([
	'add_entity.cc',
	'align_columns.cc',
	'all_commands.cc',
	'attach.cc',
	'backup.cc',
	'command.cc',
	'dot.cc',
	'edit_entity.cc',
	'editor.cc',
	'entity_type.cc',
	'expire_entity.cc',
	'expunge.cc',
	'find_entities.cc',
	'find_machines.cc',
	'ggg_main.cc',
	'guile.cc',
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
	include_directories: src,
	dependencies: [unistdx,ggg_crypt,ggg_core,libacl,guile,ggg_guile],
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

run_target(
	'form-cli',
	command: [
		'env',
		'LD_LIBRARY_PATH=' + guile_load_compiled_path,
		'GUILE_LOAD_PATH=' + guile_load_path,
		'GUILE_LOAD_COMPILED_PATH=' + guile_load_compiled_path,
		'GUILE_AUTO_COMPILE=0',
		find_program('guile2.2').path(),
		'-s',
		join_paths(meson.current_source_dir(), 'form-cli.scm')
	],
	depends: ggg_guile_forms
)

# guile_config {{{
guile_config = configuration_data()
guile_config.set('load_compiled_path', guile_load_compiled_path)
guile_config.set('load_path', guile_load_path)
configure_file(
	input: 'guile.in',
	output: 'guile',
	configuration: guile_config
)
# }}}
