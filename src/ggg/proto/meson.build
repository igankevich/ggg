ggg_proto_src = files('''
authentication.cc
connection.cc
local_server.cc
pipeline.cc
protocol.cc
remote_client.cc
result.cc
selection.cc
'''.split())

ggg_proto_src += password_src

ggg_proto_deps = [ggg_core,libsodium,zxcvbn]

ggg_proto_lib = shared_library(
	'ggg-proto',
	sources: ggg_proto_src,
	include_directories: src,
	dependencies: ggg_proto_deps,
	version: meson.project_version(),
	install: true
)

ggg_proto = declare_dependency(
	link_with: [ggg_proto_lib],
	include_directories: src,
	dependencies: ggg_proto_deps
)

ggg_proto_test_src = files(['daemon_environment.cc'])

ggg_proto_test_deps = [ggg_proto,dependency('threads')]

ggg_proto_test_lib = shared_library(
    'ggg-proto-test',
    sources: ggg_proto_test_src,
	include_directories: src,
    dependencies: ggg_proto_test_deps,
)

ggg_proto_test = declare_dependency(
    link_with: [ggg_proto_test_lib],
	include_directories: src,
	dependencies: ggg_proto_test_deps,
)
