configure_file(
	input: 'default_config.hh.in',
	output: 'default_config.hh',
	configuration: config
)

# override default configuration for integration tests
if with_integration_tests
    more_config = configuration_data()
    more_config.merge_from(config)
    more_config.set('module_name', 'test_ggg')
    more_config.set('ggg_root', meson.build_root())
    more_config.set('bind_address', '/tmp/.GGG-TEST')
    more_config.set('guile_load_path', join_paths(meson.build_root(), 'src', 'ggg', 'guile', 'test'))
    more_config.set('guile_load_compiled_path', '')
    configure_file(
        input: 'default_config.hh.in',
        output: 'test_config.hh',
        configuration: more_config
    )
endif

if with_tests
	subdir('test')
endif

subdir('core')
subdir('sec')
subdir('proto')
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
