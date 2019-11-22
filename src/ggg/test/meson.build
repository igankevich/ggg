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

has_override_fs = libdl.found()

if has_override_fs
    override_fs_config = configuration_data()
    foreach f : ['stat', 'lstat', 'fstat', 'fstatat',
        '__xstat', '__lxstat', '__fxstat', '__fxstatat']
        if cpp.has_function(f)
            override_fs_config.set('HAVE_' + f.to_upper(), 1)
        endif
    endforeach
    override_fs_config.set('workdir', meson.build_root())
    override_fs_config.set(
        'ggg_guile_prefix',
        join_paths(get_option('prefix'), get_option('libdir'))
    )
    override_fs_config.set(
        'guile_load_path',
        join_paths(meson.build_root(), 'src', 'ggg', 'guile')
    )
    configure_file(
        input: 'override_fs.hh.in',
        output: 'override_fs.hh',
        configuration: override_fs_config
    )
    override_fs = shared_library(
        'override-fs',
        sources: 'override_fs.cc',
        include_directories: src,
        dependencies: libdl
    )
endif
