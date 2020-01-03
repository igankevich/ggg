ggg_src = files('''
authentication.cc
byte_buffer.cc
connection.cc
local_server.cc
pipeline.cc
protocol.cc
remote_client.cc
gggd_main.cc
'''.split())

ggg_daemon_exe = executable(
    'gggd',
    sources: ggg_src,
    include_directories: src,
    dependencies: [unistdx,ggg_core],
    install: true
)

