ggg_core_src = files([
	'account.cc',
	'account_flags.cc',
	'database.cc',
	'entity.cc',
	'group.cc',
	'host_address.cc',
	'host.cc',
	'ip_address.cc',
	'machine.cc',
	'message.cc',
	'native.cc',
])

ggg_core_deps = [unistdx,sqlitex,sqlite]

ggg_core_lib = shared_library(
	'ggg-core',
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

test(
	'database-test',
	executable(
		'database-test',
		sources: ['database_test.cc'],
		include_directories: src,
		dependencies: [unistdx, gtest, ggg_core]
	),
	suite: 'unit'
)

