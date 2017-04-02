configure_file(
	input: 'config.hh.in',
	output: 'config.hh',
	configuration: config
)

hierarchy_src = [
	'hierarchy.cc',
	'entity.cc',
	'group.cc'
]

libnss_hierarchy = shared_library(
	'nss_' + module_name,
	sources: hierarchy_src + ['pwd.cc', 'grp.cc'],
	dependencies: [unistdx],
	version: '2',
	install: true
)

executable(
	module_name,
	sources: hierarchy_src + ['hierarchy_main.cc'],
	dependencies: [unistdx]
)

test(
	'entity-test',
	executable(
		'entity-test',
		sources: hierarchy_src + ['entity_test.cc'],
		dependencies: [unistdx, gtest]
	)
)

test(
	'hierarchy-test',
	executable(
		'hierarchy-test',
		sources: hierarchy_src + ['hierarchy_test.cc'],
		dependencies: [unistdx, gtest]
	)
)

test(
	'pwd-test',
	executable(
		'pwd-test',
		sources: hierarchy_src + ['pwd_test.cc'],
		dependencies: [unistdx, gtest],
		cpp_args: ['-DHIERARCHY_TEST']
	),
	env: [
		'LD_PRELOAD=libnss_wrapper.so',
		'NSS_WRAPPER_MODULE_SO_PATH=' + libnss_hierarchy.full_path(),
		'NSS_WRAPPER_MODULE_FN_PREFIX=' + module_name
	]
)
