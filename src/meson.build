configure_file(
	input: 'config.hh.in',
	output: 'config.hh',
	configuration: config
)

shared_library(
	'nss_' + module_name + '.so.2',
	sources: 'pwd.cc',
	dependencies: [unistdx]
)

test(
	'pwd-test',
	executable(
		'pwd-test',
		sources: 'pwd_test.cc',
		dependencies: [unistdx]
	)
)
