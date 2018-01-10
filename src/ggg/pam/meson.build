pam_src = files([
	'pam_ggg.cc',
	'pam_handle.cc',
	'pam_category.cc',
	'conversation.cc',
]) + forms_src

pam_ggg = shared_library(
	'pam_' + module_name,
	sources: pam_src,
	include_directories: inc,
	dependencies: [pam, unistdx, ggg_core, libcrypt, zxcvbn],
)

meson.add_install_script(
	join_paths(meson.source_root(), 'scripts', 'install-pam-ggg'),
	join_paths(meson.current_build_dir(), pam_ggg.full_path()),
	join_paths(get_option('libdir'), 'security'),
	'pam_' + module_name + '.so'
)
