ggg_proto_src = files('''
authentication.cc
connection.cc
local_server.cc
pipeline.cc
protocol.cc
remote_client.cc
selection.cc
'''.split())

ggg_proto_src += password_src

foreach config : binary_configs
    tmp_ggg_proto_deps = [get_variable(config.get('prefix') + 'ggg_core'),libsodium,zxcvbn]
    tmp_ggg_proto_lib = shared_library(
        config.get('prefix') + 'ggg-proto',
        sources: ggg_proto_src,
        cpp_args: config.get('cpp_args'),
        include_directories: src,
        dependencies: tmp_ggg_proto_deps,
        version: meson.project_version(),
        install: true
    )
    tmp_ggg_proto = declare_dependency(
        link_with: [tmp_ggg_proto_lib],
        include_directories: src,
        dependencies: tmp_ggg_proto_deps
    )
    set_variable(config.get('prefix') + 'ggg_proto_lib', tmp_ggg_proto_lib)
    set_variable(config.get('prefix') + 'ggg_proto', tmp_ggg_proto)
endforeach

if with_integration_tests
    ggg_proto_test_src = files(['daemon_environment.cc'])
    ggg_proto_test_deps = [test_ggg_proto,dependency('threads')]
    ggg_proto_test_lib = shared_library(
        'ggg-proto-test',
        sources: ggg_proto_test_src,
        cpp_args: config.get('cpp_args'),
        include_directories: src,
        dependencies: ggg_proto_test_deps,
    )
    ggg_proto_test = declare_dependency(
        link_with: [ggg_proto_test_lib],
        include_directories: src,
        dependencies: ggg_proto_test_deps,
    )
endif
