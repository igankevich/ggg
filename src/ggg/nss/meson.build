nss_ggg_internal_src = files([
    'group_internal.cc',
    'passwd_internal.cc',
    'shadow_internal.cc',
])

nss_ggg_src = files([
	'ethers.cc',
	'group.cc',
	'hosts.cc',
	'passwd.cc',
	'shadow.cc',
])

foreach config : binary_configs
    tmp_nss_ggg_internal_lib = static_library(
        config.get('prefix') + 'nss_ggg_internal',
        sources: nss_ggg_internal_src,
        cpp_args: config.get('cpp_args'),
        include_directories: src,
        dependencies: [unistdx, get_variable(config.get('prefix') + 'ggg_core')],
    )
    tmp_nss_ggg_lib = shared_library(
        'nss_' + config.get('prefix') + 'ggg',
        sources: nss_ggg_src,
        include_directories: src,
        link_with: [tmp_nss_ggg_internal_lib],
        dependencies: [
            get_variable(config.get('prefix') + 'ggg_core'),
            get_variable(config.get('prefix') + 'ggg_proto'),
        ],
        version: '2',
        install: true,
        cpp_args: cpp_args_visibility + config.get('cpp_args'),
    )
    set_variable(config.get('prefix') + 'nss_ggg_internal_lib', tmp_nss_ggg_internal_lib)
    set_variable(config.get('prefix') + 'nss_ggg_lib', tmp_nss_ggg_lib)
endforeach

if with_integration_tests
    override_nss_src = files(['override_nss.cc'])
    foreach service : ['passwd', 'group', 'shadow', 'hosts', 'ethers']
        name = '-'.join(['nss', service, 'test'])
        exe = executable(
            name,
            sources: [service + '_test.cc'] + override_nss_src,
            include_directories: src,
            dependencies: [unistdx, gtest, test_ggg_core, ggg_proto_test],
            link_with: [test_nss_ggg_lib,test_nss_ggg_internal_lib],
            cpp_args: ['-DGGG_NSS_SERVICE="' + service + '"'] + test_cpp_args,
        )
        test(
            name,
            exe,
            is_parallel: false,
            env: ['LD_LIBRARY_PATH=' + meson.current_build_dir()],
        )
    endforeach
    test(
        'nss-stress-test',
        executable(
            'nss-stress-test',
            sources: ['stress_test.cc'] + override_nss_src,
            include_directories: src,
            dependencies: [unistdx, gtest, test_ggg_core, ggg_proto_test],
            link_with: [test_nss_ggg_lib,test_nss_ggg_internal_lib],
            cpp_args: ['-DGGG_NSS_SERVICE="passwd"'] + test_cpp_args,
        ),
        is_parallel: false,
        env: ['LD_LIBRARY_PATH=' + meson.current_build_dir()],
    )
endif

