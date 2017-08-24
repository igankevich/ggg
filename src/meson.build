configure_file(
	input: 'config.hh.in',
	output: 'config.hh',
	configuration: config
)

build_date = vcs_tag(
	input: 'build_date.hh.in',
	output: 'build_date.hh',
	command: ['git', 'log', '-1', '--format=%cd', '--date=iso-strict'],
	fallback: 'unknown date'
)

version = vcs_tag(
	input: 'version.hh.in',
	output: 'version.hh',
	command: ['git', 'rev-parse', 'HEAD'],
	fallback: meson.project_version()
)

hierarchy_src = files([
	'hierarchy_instance.cc'
])

forms_src = files([
])

inc = include_directories('.')
pam_inc = include_directories('pam')

subdir('core')
subdir('sec')
subdir('ctl')
subdir('nss')
subdir('pam')
subdir('ggg')
subdir('test')
