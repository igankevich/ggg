ggg_guile_src = files([
	'account.cc',
	'entity.cc',
	'group.cc',
	'guile_traits.cc',
	'init.cc',
	'machine.cc',
	'message.cc',
	'password.cc',
	'public_key.cc',
	'store.cc',
	'unistdx.cc',
]) + password_src


foreach config : binary_configs
    tmp_ggg_guile_deps = [unistdx,get_variable(config.get('prefix')+'ggg_core'),
                          guile,zxcvbn,libsodium]
    tmp_ggg_guile_lib = shared_library(
        config.get('prefix') + 'ggg-guile',
        sources: ggg_guile_src,
        cpp_args: config.get('cpp_args'),
        include_directories: src,
        dependencies: tmp_ggg_guile_deps,
        version: meson.project_version(),
        install: config.get('prefix') == '',
    )
    tmp_ggg_guile = declare_dependency(
        link_with: [tmp_ggg_guile_lib],
        include_directories: src,
        dependencies: tmp_ggg_guile_deps
    )
    set_variable(config.get('prefix') + 'ggg_guile_lib', tmp_ggg_guile_lib)
    set_variable(config.get('prefix') + 'ggg_guile', tmp_ggg_guile)
endforeach

guile_load_path = ':'.join([meson.current_source_dir(),meson.current_build_dir()])
guile_load_compiled_path = meson.current_build_dir()
guile_libdir = meson.current_build_dir()
subdir('ggg')
subdir('test')
