configure_file(
	input: 'config.hh.in',
	output: 'config.hh',
	configuration: config
)

hierarchy_src = files([
	'hierarchy.cc',
	'entity.cc',
	'group.cc',
	'account.cc',
	'account_flags.cc',
	'hierarchy_instance.cc',
	'shred.cc'
])

nss_src = files([
	'pwd.cc',
	'grp.cc',
	'shadow.cc'
])

libnss_hierarchy = shared_library(
	'nss_' + module_name,
	sources: hierarchy_src + nss_src,
	dependencies: [unistdx],
	version: '2',
	install: true
)

inc = include_directories('.')

subdir('test')
subdir('pam')
