project(
	'ggg',
	'cpp',
	version: '0.4',
	meson_version: '>=0.31',
	default_options: ['cpp_std=c++14']
)

cpp = meson.get_compiler('cpp')
if not cpp.has_header('nss.h')
	error('Header nss.h is required to compile the module.')
endif

if not get_option('buildtype').contains('debug')
	add_global_arguments('-DNDEBUG', language: 'cpp')
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
config.set('lock_directory', join_paths(etc, module_name, 'lck'))
config.set('salt_length', '16')
config.set('executable_name', executable_name)
config.set('version', meson.project_version())
config.set('file_editor', 'vi')
config.set('directory_editor', 'vidir')
config.set('catalog', 'ggg')

unistdx = dependency('unistdx')
gtest = dependency('gtest', main: true)
gtest_nomain = dependency('gtest', main: false)
nss_wrapper = dependency('nss_wrapper')
threads = dependency('threads')
zxcvbn = dependency('zxcvbn')
pam = cpp.find_library('pam')
libcrypt = cpp.find_library('crypt')

# integration tests {{{
unshare = find_program('unshare', required: false)
su = find_program('su', required: false)
if unshare.found() and su.found()
	run_target(
		'init-integration-tests',
		command: join_paths(
			meson.source_root(),
			'scripts',
			'init-integration-tests'
		)
	)
endif
# }}}

subdir('src')
subdir('po')

# archive {{{
archive_name = meson.project_name() + '-' + meson.project_version()
run_target(
	'archive',
	command: [
		'git',
		'archive',
		'-o',
		join_paths(meson.build_root(), archive_name + '.tar.xz'),
		'--prefix',
		archive_name + '/',
		'HEAD'
	]
)
# }}}
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
