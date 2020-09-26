ggg_core_src = files([
	'account.cc',
	'account_flags.cc',
	'database.cc',
	'entity.cc',
	'group.cc',
	'host_address.cc',
	'host.cc',
	'ip_address.cc',
	'machine.cc',
	'message.cc',
	'native.cc',
	'public_key.cc',
])

ggg_core_deps = [unistdx,sqlitex]

foreach config : binary_configs
    tmp_ggg_core_lib = shared_library(
        config.get('prefix') + 'ggg-core',
        sources: ggg_core_src,
        cpp_args: config.get('cpp_args'),
        include_directories: src,
        dependencies: ggg_core_deps,
        version: meson.project_version(),
        install: true
    )
    tmp_ggg_core = declare_dependency(
        link_with: [tmp_ggg_core_lib],
        include_directories: src,
        dependencies: ggg_core_deps
    )
    set_variable(config.get('prefix') + 'ggg_core_lib', tmp_ggg_core_lib)
    set_variable(config.get('prefix') + 'ggg_core', tmp_ggg_core)
endforeach

foreach name : ['entity', 'group', 'account']
    exe = executable(
        name + '-test',
        sources: [name + '_test.cc'],
        include_directories: src,
        dependencies: [unistdx, gtest, ggg_core]
    )
    test('core/' + name, exe)
endforeach

if with_integration_tests
    name = 'database'
    exe = executable(
        name + '-test',
        sources: [name + '_test.cc'],
        include_directories: src,
        dependencies: [unistdx, gtest, test_ggg_core],
    )
    test('core/' + name, exe, workdir: meson.build_root())
endif
