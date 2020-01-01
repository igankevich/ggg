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
    cpp_args: cpp_args_visibility,
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
	dependencies: [unistdx,ggg_core],
	version: '2',
	install: true,
    cpp_args: cpp_args_visibility,
)

if has_override_fs
    override_nss_src = files(['override_nss.cc'])
	foreach service : ['passwd', 'group', 'shadow', 'hosts']
		name = '-'.join(['nss', service, 'test'])
        exe = executable(
            name,
            sources: [service + '_test.cc'] + override_nss_src,
            include_directories: src,
            dependencies: [unistdx, gtest, ggg_core],
            link_with: [nss_ggg_lib,nss_ggg_internal_lib],
            cpp_args: ['-DGGG_NSS_SERVICE="' + service + '"']
        )
		test(
            name,
            exe,
            env: [
                'LD_PRELOAD=' + override_fs.full_path(),
                'GGG_TEST_SUFFIX='+name
            ]
		)
	endforeach
endif

