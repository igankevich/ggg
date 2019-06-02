ggg_core_src = files([
	'account.cc',
	'account_flags.cc',
	'database.cc',
	'entity.cc',
	'form.cc',
	'form_field.cc',
	'form_type.cc',
	'group.cc',
	'host_address.cc',
	'host.cc',
	'ip_address.cc',
	'machine.cc',
	'native.cc',
	'shred.cc'
])

ggg_core_deps = [unistdx,sqlitex,sqlite]

ggg_core_lib = shared_library(
	'ggg_core',
	sources: ggg_core_src,
	include_directories: src,
	dependencies: ggg_core_deps,
	version: meson.project_version(),
	install: true
)

ggg_core = declare_dependency(
	link_with: [ggg_core_lib],
	include_directories: src,
	dependencies: ggg_core_deps
)
