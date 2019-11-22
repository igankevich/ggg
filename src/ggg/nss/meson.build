nss_src = files([
	'ethers.cc',
	'group.cc',
	'hosts.cc',
	'passwd.cc',
	'shadow.cc',
])

libnss_ggg = shared_library(
	'nss_' + module_name,
	sources: nss_src,
	include_directories: src,
	dependencies: [unistdx,ggg_core],
	version: '2',
	install: true
)

if has_override_fs
    override_nss_src = files(['override_nss.cc'])
	foreach service : ['passwd', 'group', 'shadow', 'hosts']
		name = '-'.join(['nss', service, 'test'])
        exe = executable(
            name,
            sources: ['nss_' + service + '_test.cc'] + override_nss_src,
            include_directories: src,
            dependencies: [unistdx, gtest, ggg_core],
            link_with: [libnss_ggg,override_fs],
            cpp_args: ['-DGGG_NSS_SERVICE="' + service + '"']
        )
		test(name, exe, env: ['LD_PRELOAD=' + override_fs.full_path()],
            is_parallel: false)
	endforeach
endif

