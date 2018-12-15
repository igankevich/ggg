project(
	'ggg',
	'cpp',
	version: '0.10.9',
	meson_version: '>=0.46',
	default_options: ['cpp_std=c++14'],
	license: ['GPLv2']
)

cpp = meson.get_compiler('cpp')
if not cpp.has_header('nss.h')
	error('Header nss.h is required to compile the module.')
endif

debug_build = get_option('buildtype').contains('debug')
if not debug_build
	add_global_arguments('-DNDEBUG', language: 'cpp')
else
	add_global_link_arguments('-rdynamic', language: 'cpp')
endif

datadir = join_paths(
	get_option('prefix'),
	get_option('datadir'),
	meson.project_name()
)

module_name = 'ggg'
executable_name = 'ggg'
etc = get_option('sysconfdir')
config = configuration_data()
config.set('module_name', module_name)
config.set('root_directory', join_paths(etc, module_name))
config.set('ent_root_directory', join_paths(etc, module_name, 'ent'))
config.set('reg_root_directory', join_paths(etc, module_name, 'reg'))
config.set('git_root_directory', join_paths(etc, module_name, '.git'))
config.set('shadow_file', join_paths(etc, module_name, 'acc', 'shadow'))
config.set('shadow_file_new', join_paths(etc, module_name, 'acc', '.shadow.new'))
config.set('lock_file', join_paths(etc, module_name, '.lock'))
config.set('salt_length', '16')
config.set('executable_name', executable_name)
config.set('version', debug_build ? 'debug' : meson.project_version())
config.set('file_editor', 'vi')
config.set('directory_editor', 'vidir')
config.set('catalog', 'ggg')
config.set('cache_directory', join_paths(get_option('localstatedir'), 'cache'))
config.set('read_group', 'ggg.auth')
config.set('write_group', 'ggg.write')

unistdx = dependency('unistdx', version: '>=0.4.6')
gtest = dependency('gtest', main: true)
gtest_nomain = dependency('gtest', main: false)
nss_wrapper = dependency('nss_wrapper')
threads = dependency('threads')
zxcvbn = dependency('zxcvbn')
pam = cpp.find_library('pam')
libcrypt = cpp.find_library('crypt')
libacl = cpp.find_library('acl')

subdir('src')
subdir('po')
subdir('pkg')

run_target(
	'init-integration-tests',
	command: join_paths(meson.source_root(), 'scripts', 'init-integration-tests')
)
# cppcheck {{{
cppcheck = find_program('cppcheck', required: false)
if cppcheck.found()
	run_target(
		'cppcheck',
		command: [
			cppcheck,
			'--enable=all',
			'--quiet',
			'--force',
			'--language=c++',
			'--std=c++11',
			'--template=gcc',
			'-I' + join_paths(meson.source_root(), 'src'),
			'-I' + join_paths(meson.build_root(), 'src'),
			join_paths(meson.source_root(), 'src'),
		]
	)
endif
# }}}
# gettext {{{
run_target('po', command: [join_paths(meson.source_root(), 'scripts', 'po')])
# }}}
