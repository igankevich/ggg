configure_file(
	input: 'config.hh.in',
	output: 'config.hh',
	configuration: config
)

hierarchy_src = files([
])

forms_src = files([
])

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
