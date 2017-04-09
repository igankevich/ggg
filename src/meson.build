configure_file(
	input: 'config.hh.in',
	output: 'config.hh',
	configuration: config
)

hierarchy_src = [
	'hierarchy.cc',
	'entity.cc',
	'group.cc',
	'hierarchy_instance.cc'
]

libnss_hierarchy = shared_library(
	'nss_' + module_name,
	sources: hierarchy_src + ['pwd.cc', 'grp.cc'],
	dependencies: [unistdx],
	version: '2',
	install: true
)

libnss_hierarchy_test = shared_library(
	'nss_' + module_name + '_test',
	sources: hierarchy_src + ['pwd.cc', 'grp.cc'],
	dependencies: [unistdx],
	version: '2',
	cpp_args: ['-DHIERARCHY_TEST']
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
		'NSS_WRAPPER_MODULE_SO_PATH=' + libnss_hierarchy_test.full_path(),
		'NSS_WRAPPER_MODULE_FN_PREFIX=' + module_name,
		'NSS_WRAPPER_DEBUGLEVEL=3',
		'NSS_WRAPPER_PASSWD=passwd',
		'NSS_WRAPPER_GROUP=group'
	]
)
