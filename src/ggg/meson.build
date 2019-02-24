configure_file(
	input: 'config.hh.in',
	output: 'config.hh',
	configuration: config
)

subdir('core')
subdir('ctl')

if with_nss
	subdir('nss')
endif

if with_pam
	subdir('pam')
endif

if with_cli
	subdir('cli')
endif

if with_gtk3
	subdir('regform')
endif

if with_tests
	subdir('test')
endif
