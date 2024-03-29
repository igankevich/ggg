project(
	'ggg',
	'cpp',
	version: '1.11.6',
	meson_version: '>=0.50',
	default_options: ['cpp_std=c++11'],
	license: ['gpl3+']
)

cpp = meson.get_compiler('cpp')
if not cpp.has_header('nss.h')
	error('Header nss.h is required to compile the module.')
endif

cpp_args = [
    '-Werror=return-type',
    '-Werror=return-local-addr',
    '-Werror=cast-align',
    '-Wcast-align=strict',
    '-fvisibility-inlines-hidden'
]

cpp_link_args = [
    '-rdynamic'
]

foreach arg : cpp_args
	if cpp.has_argument(arg)
		add_global_arguments(arg, language: 'cpp')
	endif
endforeach

foreach arg : cpp_link_args
	if cpp.has_link_argument(arg)
		add_global_link_arguments(arg, language: 'cpp')
	endif
endforeach

debug_build = get_option('buildtype').contains('debug')
if not debug_build
	add_global_arguments('-DNDEBUG', language: 'cpp')
endif

datadir = join_paths(get_option('prefix'), get_option('datadir'), meson.project_name())

with_nss = get_option('with_nss')
with_pam = get_option('with_pam')
with_cli = get_option('with_cli')
with_daemon = get_option('with_daemon')
with_tests = get_option('with_tests')
with_integration_tests = get_option('with_integration_tests')
with_man = get_option('with_man')
b_sanitize = get_option('b_sanitize')
developer = get_option('developer')

test_cpp_args = ['-DGGG_TEST']
binary_configs = [{'prefix': '', 'cpp_args': []}]
if with_integration_tests
    binary_configs += [{'prefix': 'test_', 'cpp_args': test_cpp_args}]
endif

unistdx_version = '>= 0.19.10'
sqlitex_version = '>= 0.4.6'

unistdx = dependency('unistdx', version: unistdx_version)
zxcvbn = cpp.find_library('zxcvbn')
pam = cpp.find_library('pam')
sqlitex = dependency('sqlitex', version: sqlitex_version)
guile = dependency('guile-3.0')
libsodium = dependency('libsodium')
gtest = dependency('gtest', main: true)
gtest_nomain = dependency('gtest', main: false)
libdl = cpp.find_library('dl', required: false)

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
ggg_root = join_paths(state_dir, module_name)
if not ggg_root.startswith('/')
    ggg_root = join_paths(get_option('prefix'), ggg_root)
endif
config = configuration_data()
config.set('module_name', module_name)
config.set('executable_name', executable_name)
config.set('version', debug_build ? 'debug' : meson.project_version())
config.set('file_editor', get_option('default_editor'))
config.set('catalog', 'ggg')
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
config.set('GGG_DEVELOPER_BUILD', developer)
config.set('ggg_root', ggg_root)
config.set('bind_address', '/tmp/.GGG')
config.set('ggg_client_conf', '/etc/ggg/client.conf')
visibility_main = '[[gnu::visibility("default")]] int func(); int main() {}'
if cpp.has_argument('-fvisibility=hidden') and cpp.compiles(visibility_main)
    cpp_args_visibility = ['-fvisibility=hidden']
    config.set('GGG_HAVE_VISIBILITY', 1)
else
    cpp_args_visibility = []
endif

subdir('src')
subdir('po')
subdir('guix')

if with_man
	subdir('man')
endif

# env {{{
env = configuration_data()
env.set('build_root', meson.build_root())
env.set('prefix', join_paths(meson.build_root(), 'usr'))
configure_file(input: 'env.in', output: 'env', configuration: env)
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
