project(
	'ggg',
	'cpp',
	version: '0.1',
	meson_version: '>=0.31',
	default_options: ['cpp_std=c++11']
)

cpp = meson.get_compiler('cpp')
if not cpp.has_header('nss.h')
	error('Header nss.h is required to compile the module.')
endif

module_name = 'ggg'
config = configuration_data()
config.set('module_name', module_name)
config.set('root_directory', '/etc/' + module_name)

unistdx = dependency('unistdx')
gtest = dependency('gtest', main: true)
nss_wrapper = dependency('nss_wrapper')

subdir('src')
