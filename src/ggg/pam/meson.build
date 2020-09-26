ggg_pam_internal_src = files([
	'pam_category.cc',
	'conversation.cc',
	'handle.cc',
])

ggg_pam_src = files([
	'pam_ggg.cc',
	'pam_handle.cc',
])

foreach config : binary_configs
    tmp_ggg_pam_deps = [
        pam,
        unistdx,
        get_variable(config.get('prefix') + 'ggg_core'),
        get_variable(config.get('prefix') + 'ggg_proto'),
    ]
    tmp_ggg_pam_internal_lib = static_library(
        config.get('prefix') + 'pam_ggg_internal',
        sources: ggg_pam_internal_src,
        include_directories: src,
        dependencies: tmp_ggg_pam_deps,
        cpp_args: config.get('cpp_args') + cpp_args_visibility,
    )
    tmp_ggg_pam_lib = shared_library(
        config.get('prefix') + 'pam_ggg',
        sources: ggg_pam_src,
        include_directories: src,
        dependencies: tmp_ggg_pam_deps,
        link_with: [tmp_ggg_pam_internal_lib],
        cpp_args: config.get('cpp_args') + cpp_args_visibility,
    )
    tmp_ggg_pam = declare_dependency(
        link_with: [tmp_ggg_pam_lib],
        include_directories: src,
        dependencies: tmp_ggg_pam_deps
    )
    set_variable(config.get('prefix') + 'ggg_pam_lib', tmp_ggg_pam_lib)
    set_variable(config.get('prefix') + 'ggg_pam_internal_lib', tmp_ggg_pam_internal_lib)
    set_variable(config.get('prefix') + 'ggg_pam', tmp_ggg_pam)
endforeach

meson.add_install_script(
	join_paths(meson.source_root(), 'scripts', 'install-pam-ggg'),
	join_paths(meson.current_build_dir(), ggg_pam_lib.full_path()),
	join_paths(get_option('libdir'), 'security'),
	'pam_' + module_name + '.so'
)

test(
	'conversation-test',
	executable(
		'conversation-test',
		sources: ['conversation_test.cc'],
		include_directories: src,
		dependencies: [unistdx, gtest, libsodium, pam, ggg_core, ggg_pam]
	)
)

pam_wrapper = dependency('pam_wrapper', required: false)
pam_wrapper_service_dir = join_paths(meson.current_build_dir(), 'pam.d')
if pam_wrapper.found() and with_integration_tests
    ld_preload_items = ['libpam_wrapper.so']
    if b_sanitize == 'address'
        ld_preload_items = ['libasan.so.2'] + ld_preload_items
    endif
    ld_preload = 'LD_PRELOAD=' + ' '.join(ld_preload_items)
    test(
        'pam-ggg-test',
        executable(
            'pam-ggg-test',
            sources: ['pam_ggg_test.cc'],
            include_directories: src,
            dependencies: [unistdx, gtest, ggg_pam, ggg_proto_test],
            link_with: [test_ggg_pam_internal_lib],
        ),
        env: [
            ld_preload,
            'PAM_WRAPPER=1',
            'PAM_WRAPPER_DEBUGLEVEL=0',
            'PAM_WRAPPER_SERVICE_DIR=' + pam_wrapper_service_dir,
            'GGG_TEST_SUFFIX=pam-ggg-test'
        ],
        timeout: 120
    )
    subdir('pam.d')
endif


