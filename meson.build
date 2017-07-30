project(
	'ggg',
	'cpp',
	version: '0.1',
	meson_version: '>=0.31',
	default_options: ['cpp_std=c++14']
)

cpp = meson.get_compiler('cpp')
if not cpp.has_header('nss.h')
	error('Header nss.h is required to compile the module.')
endif

module_name = 'ggg'
executable_name = 'ggg'
config = configuration_data()
config.set('module_name', module_name)
config.set('root_directory', '/etc/' + module_name)
config.set('shadow_file', '/etc/' + module_name + '/shadow')
config.set('shadow_file_new', '/etc/' + module_name + '/.shadow.new')
config.set('salt_length', '16')
config.set('executable_name', executable_name)

unistdx = dependency('unistdx')
gtest = dependency('gtest', main: true)
nss_wrapper = dependency('nss_wrapper')
pam = cpp.find_library('pam')
libcrypt = cpp.find_library('crypt')

subdir('src')
