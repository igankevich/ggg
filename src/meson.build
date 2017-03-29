configure_file(
	input: 'config.hh.in',
	output: 'config.hh',
	configuration: config
)

hierarchy_src = [
	'hierarchy.cc',
	'entity.cc'
]

shared_library(
	'nss_' + module_name + '.so.2',
	sources: hierarchy_src + ['pwd.cc'],
	dependencies: [unistdx]
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
		sources: ['entity.cc', 'entity_test.cc'],
		dependencies: [unistdx, gtest]
	)
)
