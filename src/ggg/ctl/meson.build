hierarchy_src += files([
	'account_ctl.cc',
	'ggg.cc',
])

forms_src += files([
	'form.cc',
	'password.cc',
])

ggg_core_deps = [unistdx,libacl,sqlitex,sqlite]

ggg_core_lib = shared_library(
	'ggg_core',
	sources: hierarchy_src,
	include_directories: inc,
	dependencies: ggg_core_deps,
	version: meson.project_version(),
	install: true
)

ggg_core = declare_dependency(
	link_with: [ggg_core_lib],
	include_directories: inc,
	dependencies: ggg_core_deps
)
