ggg_guile_src = files([
	'account.cc',
	'entity.cc',
	'form.cc',
	'group.cc',
	'guile_traits.cc',
	'init.cc',
	'machine.cc',
	'password.cc',
	'store.cc',
]) + password_src

ggg_guile_deps = [unistdx,ggg_core,guile,zxcvbn,ggg_crypt]

ggg_guile_lib = shared_library(
	'ggg-guile',
	sources: ggg_guile_src,
	include_directories: src,
	dependencies: ggg_guile_deps,
	version: meson.project_version(),
	install: true,
	install_dir: guile.get_pkgconfig_variable('extensiondir')
)

ggg_guile = declare_dependency(
	link_with: [ggg_guile_lib],
	include_directories: src,
	dependencies: ggg_guile_deps
)

guile_load_path = meson.current_source_dir()
guile_load_compiled_path = meson.current_build_dir()
subdir('ggg')
