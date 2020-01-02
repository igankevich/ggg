ggg_src = files('''
connection.cc
local_server.cc
pipeline.cc
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

