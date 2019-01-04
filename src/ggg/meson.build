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

if with_nss
	subdir('nss')
endif

if with_pam
	subdir('pam')
endif

subdir('cli')

if with_gtk3
	subdir('regform')
endif

if with_tests
	subdir('test')
endif
