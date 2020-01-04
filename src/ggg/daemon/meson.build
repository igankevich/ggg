ggg_src = files('''
main.cc
'''.split())

ggg_daemon_exe = executable(
    'ggg-daemon',
    sources: ggg_src,
    include_directories: src,
    dependencies: [unistdx,ggg_core,ggg_proto],
    install: true
)
