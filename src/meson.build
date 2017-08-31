configure_file(
	input: 'config.hh.in',
	output: 'config.hh',
	configuration: config
)

hierarchy_src = files([
	'hierarchy_instance.cc'
])

forms_src = files([
])

inc = include_directories('.')
pam_inc = include_directories('pam')

subdir('core')
subdir('sec')
subdir('ctl')
subdir('nss')
subdir('pam')
subdir('ggg')
if get_option('gui')
	subdir('regform')
endif
subdir('test')
