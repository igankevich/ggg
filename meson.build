project(
	'ggg',
	'cpp',
	version: '1.1.1',
	meson_version: '>=0.46',
	default_options: ['cpp_std=c++11'],
	license: ['GPLv2']
)

cpp = meson.get_compiler('cpp')
if not cpp.has_header('nss.h')
	error('Header nss.h is required to compile the module.')
endif

foreach arg : ['-Werror=return-type']
	if cpp.has_argument(arg)
		add_global_arguments(arg, language: 'cpp')
	endif
endforeach

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

with_nss = get_option('with_nss')
with_pam = get_option('with_pam')
with_tests = get_option('with_tests')
with_gtk3 = get_option('with_gtk3')
with_man = get_option('with_man')
b_sanitize = get_option('b_sanitize')

module_name = 'ggg'
executable_name = 'ggg'
etc = get_option('sysconfdir')
config = configuration_data()
config.set('module_name', module_name)
config.set('root_directory', join_paths(etc, module_name))
config.set('ent_root_directory', join_paths(etc, module_name, 'ent'))
config.set('reg_root_directory', join_paths(etc, module_name, 'reg'))
config.set('shadow_file', join_paths(etc, module_name, 'acc', 'shadow'))
config.set('shadow_file_new', join_paths(etc, module_name, 'acc', '.shadow.new'))
config.set('salt_length', '16')
config.set('executable_name', executable_name)
config.set('version', debug_build ? 'debug' : meson.project_version())
config.set('file_editor', 'vi')
config.set('catalog', 'ggg')
config.set('cache_directory', join_paths(get_option('localstatedir'), 'cache'))
config.set('read_group', 'ggg.auth')
config.set('write_group', 'ggg.write')
config.set('max_depth', 1000)
config.set('entities_path', join_paths(etc, module_name, 'entities.sqlite3'))
config.set('accounts_path', join_paths(etc, module_name, 'accounts.sqlite3'))
config.set('min_id', 1000)
config.set('overflow_id', 65534)

unistdx_version = '>= 0.5.4'
ggg_crypt_version = '>= 0.1.1'

unistdx = dependency('unistdx', version: unistdx_version)
zxcvbn = dependency('zxcvbn')
pam = cpp.find_library('pam')
ggg_crypt = dependency('ggg-crypt', version: ggg_crypt_version)
libacl = cpp.find_library('acl')
sqlite = dependency('sqlite3')
sqlitex = dependency('sqlitex')

subdir('src')
subdir('po')
subdir('pkg')

if with_man
	subdir('man')
endif

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
