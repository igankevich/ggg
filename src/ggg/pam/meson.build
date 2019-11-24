ggg_pam_src = files([
	'pam_ggg.cc',
	'pam_handle.cc',
	'pam_category.cc',
	'conversation.cc',
	'handle.cc',
]) + password_src

ggg_pam_deps = [ggg_crypt,libsodium, pam, unistdx, ggg_core, zxcvbn]

ggg_pam_lib = shared_library(
	'pam_ggg',
	sources: ggg_pam_src,
	include_directories: src,
	dependencies: ggg_pam_deps
)

ggg_pam = declare_dependency(
	link_with: [ggg_pam_lib],
	include_directories: src,
	dependencies: ggg_pam_deps
)

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
		dependencies: [unistdx, gtest, ggg_crypt, libsodium, pam, ggg_core, ggg_pam]
	)
)

pam_wrapper = dependency('pam_wrapper', required: false)
if pam_wrapper.found() and not state_dir.startswith('/') and b_sanitize != 'address'
    ld_preload_items = ['libpam_wrapper.so']
    ld_preload = 'LD_PRELOAD=' + ' '.join(ld_preload_items)
    test(
        'pam-ggg-test',
        executable(
            'pam-ggg-test',
            sources: ['pam_ggg_test.cc'],
            include_directories: src,
            dependencies: [unistdx, gtest, ggg_pam]
        ),
        env: [
            ld_preload,
            'PAM_WRAPPER=1',
            'PAM_WRAPPER_DEBUGLEVEL=3',
            'PAM_WRAPPER_SERVICE_DIR=' + join_paths(
                meson.current_build_dir(),
                'pam.d'
            )
        ],
        is_parallel: false
    )
    subdir('pam.d')
endif

