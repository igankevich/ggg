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
])

ggg_core_deps = [unistdx,sqlitex,sqlite]

ggg_core_lib = shared_library(
	'ggg-core',
	sources: ggg_core_src,
	include_directories: src,
	dependencies: ggg_core_deps,
	version: meson.project_version(),
	install: true
)

ggg_core = declare_dependency(
	link_with: [ggg_core_lib],
	include_directories: src,
	dependencies: ggg_core_deps
)

foreach name : ['entity', 'group', 'account']
    exe = executable(
        name + '-test',
        sources: [name + '_test.cc'],
        include_directories: src,
        dependencies: [unistdx, gtest, ggg_core]
    )
    test(name + '-test', exe)
endforeach

if has_override_fs
    name = 'database'
    exe = executable(
        name + '-test',
        sources: [name + '_test.cc'],
        include_directories: src,
        dependencies: [unistdx, gtest, ggg_core],
    )
    test(
        name + '-test', exe,
		env: ['LD_PRELOAD='+override_fs.full_path()],
        workdir: meson.build_root(),
        is_parallel: false,
    )
endif
