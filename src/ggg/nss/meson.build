nss_ggg_internal_src = files([
    'group_internal.cc',
    'passwd_internal.cc',
    'shadow_internal.cc',
])

nss_ggg_internal_lib = static_library(
	'nss_ggg_internal',
	sources: nss_ggg_internal_src,
	include_directories: src,
	dependencies: [unistdx,ggg_core],
)

nss_ggg_src = files([
	'ethers.cc',
	'group.cc',
	'hosts.cc',
	'passwd.cc',
	'shadow.cc',
])

nss_ggg_lib = shared_library(
	'nss_ggg',
	sources: nss_ggg_src,
	include_directories: src,
    link_with: [nss_ggg_internal_lib],
	dependencies: [ggg_core,ggg_proto],
	version: '2',
	install: true,
    cpp_args: cpp_args_visibility,
    link_args: '-Wl,--version-script=' + meson.current_source_dir() + '/libnss.version'
)

if has_override_fs
    override_nss_src = files(['override_nss.cc'])
	foreach service : ['passwd', 'group', 'shadow', 'hosts', 'ethers']
		name = '-'.join(['nss', service, 'test'])
        exe = executable(
            name,
            sources: [service + '_test.cc'] + override_nss_src,
            include_directories: src,
            dependencies: [unistdx, gtest, ggg_core, ggg_proto_test],
            link_with: [nss_ggg_lib,nss_ggg_internal_lib],
            cpp_args: ['-DGGG_NSS_SERVICE="' + service + '"']
        )
		test(
            name,
            exe,
            env: [
                'LD_PRELOAD=' + override_fs.full_path(),
                'GGG_TEST_SUFFIX='+name
            ],
            is_parallel: false
		)
	endforeach
endif

