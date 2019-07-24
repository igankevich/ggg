foreach name : ['types', 'forms']
	tmp = custom_target(
		'guile-' + name,
		input: name + '.scm',
		output: name + '.go',
		command: [
			'env',
			'LD_LIBRARY_PATH=' + guile_load_compiled_path,
			'GUILE_LOAD_PATH=' + guile_load_path,
			'GUILE_LOAD_COMPILED_PATH=' + guile_load_compiled_path,
			'GUILE_AUTO_COMPILE=0',
			find_program('guild').path(),
			'compile',
			'-o', join_paths(meson.current_build_dir(), '@BASENAME@.go'),
			'@INPUT@',
		]
	)
	set_variable('ggg_guile_' + name, tmp)
endforeach

install_data(
	['types.scm','forms.scm'],
	install_dir: join_paths(get_option('prefix'), get_option('datadir'), 'ggg')
)
