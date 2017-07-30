configure_file(
	input: 'config.hh.in',
	output: 'config.hh',
	configuration: config
)

hierarchy_src = files([
	'hierarchy_instance.cc'
])

inc = include_directories('.')

subdir('core')
subdir('sec')
subdir('nss')
subdir('pam')
subdir('ggg')
subdir('test')
