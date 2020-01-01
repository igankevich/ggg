ggg_pam_deps = [ggg_crypt, libsodium, pam, unistdx, ggg_core, zxcvbn]

ggg_pam_internal_src = files([
	'pam_category.cc',
	'conversation.cc',
	'handle.cc',
]) + password_src

ggg_pam_internal_lib = static_library(
	'pam_ggg_internal',
	sources: ggg_pam_internal_src,
	include_directories: src,
	dependencies: ggg_pam_deps,
    cpp_args: cpp_args_visibility,
)

ggg_pam_src = files([
	'pam_ggg.cc',
	'pam_handle.cc',
])

ggg_pam_lib = shared_library(
	'pam_ggg',
	sources: ggg_pam_src,
	include_directories: src,
	dependencies: ggg_pam_deps,
    link_with: [ggg_pam_internal_lib],
    cpp_args: cpp_args_visibility,
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
if pam_wrapper.found() and b_sanitize != 'address' and has_override_fs
    ld_preload_items = ['libpam_wrapper.so', override_fs.full_path()]
    ld_preload = 'LD_PRELOAD=' + ' '.join(ld_preload_items)
    test(
        'pam-ggg-test',
        executable(
            'pam-ggg-test',
            sources: ['pam_ggg_test.cc'],
            include_directories: src,
            dependencies: [unistdx, gtest, ggg_pam],
            link_with: [ggg_pam_internal_lib],
        ),
        env: [
            ld_preload,
            'PAM_WRAPPER=1',
            'PAM_WRAPPER_DEBUGLEVEL=0',
            'PAM_WRAPPER_SERVICE_DIR=' + join_paths(
                meson.current_build_dir(),
                'pam.d'
            ),
            'GGG_TEST_SUFFIX=pam-ggg-test'
        ],
        timeout: 120
    )
    subdir('pam.d')
endif

