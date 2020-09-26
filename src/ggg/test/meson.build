ggg_test_src = files(['execute_command.cc'])

ggg_test_deps = [unistdx]

ggg_test_lib = shared_library(
    'ggg-test',
    sources: ggg_test_src,
	include_directories: src,
    dependencies: ggg_test_deps,
)

ggg_test = declare_dependency(
    link_with: [ggg_test_lib],
	include_directories: src,
	dependencies: ggg_test_deps,
)

sh = find_program('sh')
test_config = configuration_data()
test_config.set('sh', sh.path())
configure_file(
    input: 'config.hh.in',
    output: 'config.hh',
    configuration: test_config
)
