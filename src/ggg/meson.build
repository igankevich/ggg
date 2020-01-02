configure_file(
	input: 'config.hh.in',
	output: 'config.hh',
	configuration: config
)

if with_tests
	subdir('test')
endif

subdir('core')
subdir('sec')
subdir('guile')

if with_nss
	subdir('nss')
endif

if with_pam
	subdir('pam')
endif

if with_cli
	subdir('cli')
endif

if with_daemon
    subdir('daemon')
endif
