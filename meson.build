project(
	'ggg',
	'cpp',
	version: '1.3.4',
	meson_version: '>=0.50',
	default_options: ['cpp_std=c++11'],
	license: ['gpl3+']
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

datadir = join_paths(get_option('prefix'), get_option('datadir'), meson.project_name())

with_nss = get_option('with_nss')
with_pam = get_option('with_pam')
with_cli = get_option('with_cli')
with_tests = get_option('with_tests')
with_man = get_option('with_man')
b_sanitize = get_option('b_sanitize')
developer = get_option('developer')

unistdx_version = '>= 0.5.4'
ggg_crypt_version = '>= 0.1.1'
sqlitex_version = '>= 0.4.4'

unistdx = dependency('unistdx', version: unistdx_version)
zxcvbn = cpp.find_library('zxcvbn')
pam = cpp.find_library('pam')
ggg_crypt = dependency('ggg-crypt', version: ggg_crypt_version)
sqlite = dependency('sqlite3')
sqlitex = dependency('sqlitex', version: sqlitex_version)
guile = dependency('guile-2.2')
libsodium = dependency('libsodium')

v = guile.version().split('.')
guile_version = '.'.join([v[0],v[1]])
site_directory = join_paths(get_option('prefix'), get_option('datadir'),
    'guile', 'site', guile_version)
scm_directory = join_paths(site_directory, 'ggg')
site_ccache_directory = join_paths(get_option('prefix'), get_option('libdir'),
    'guile', guile_version, 'site-ccache')
go_directory = join_paths(site_ccache_directory, 'ggg')

module_name = 'ggg'
executable_name = 'ggg'
state_dir = get_option('sharedstatedir')
config = configuration_data()
config.set('module_name', module_name)
config.set('state_dir', join_paths(state_dir, module_name))
config.set('salt_length', '16')
config.set('executable_name', executable_name)
config.set('version', debug_build ? 'debug' : meson.project_version())
config.set('file_editor', 'vi')
config.set('catalog', 'ggg')
config.set('read_group', 'ggg.auth')
config.set('write_group', 'ggg.write')
config.set('max_depth', 1000)
config.set('entities', 'entities.sqlite3')
config.set('accounts', 'accounts.sqlite3')
config.set('min_id', 1000)
config.set('overflow_id', 65534)
config.set('default_home_prefix', '/home')
config.set('default_shell', '/bin/sh')
config.set('guile_load_path', site_directory)
config.set('guile_load_compiled_path', site_ccache_directory)

subdir('src')
subdir('po')
subdir('pkg')

if with_man
	subdir('man')
endif

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
