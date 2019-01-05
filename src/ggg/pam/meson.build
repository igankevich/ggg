pam_src = files([
	'pam_ggg.cc',
	'pam_handle.cc',
	'pam_category.cc',
	'conversation.cc',
]) + forms_src

ggg_pam_deps = [libcrypt, pam, unistdx, ggg_core, zxcvbn]

ggg_pam_lib = shared_library(
	'pam_ggg',
	sources: pam_src,
	include_directories: inc,
	dependencies: ggg_pam_deps
)

ggg_pam = declare_dependency(
	link_with: [ggg_pam_lib],
	include_directories: inc,
	dependencies: ggg_pam_deps
)

meson.add_install_script(
	join_paths(meson.source_root(), 'scripts', 'install-pam-ggg'),
	join_paths(meson.current_build_dir(), ggg_pam_lib.full_path()),
	join_paths(get_option('libdir'), 'security'),
	'pam_' + module_name + '.so'
)
