project(
	'nss-legion',
	'cpp',
	version: '0.1',
	meson_version: '>=0.31',
	default_options: ['cpp_std=c++11']
)

cpp = meson.get_compiler('cpp')
if not cpp.has_header('nss.h')
	error('Header nss.h is required to compile the module.')
endif

module_name = 'legion'
config = configuration_data()
config.set('module_name', module_name)
config.set('root_directory', '/etc/legion')

unistdx = dependency('unistdx')

subdir('src')
