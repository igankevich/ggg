test_src = files([
	'chroot_environment.cc',
	'execute_command.cc',
])

override_nss_src = files([
	'override_nss.cc'
])

test(
	'conversation-test',
	executable(
		'conversation-test',
		sources: ['conversation_test.cc'],
		include_directories: src,
		dependencies: [unistdx, gtest, ggg_crypt,libsodium, pam, ggg_core, ggg_pam]
	),
	suite: 'unit'
)

if with_nss and not state_dir.startswith('/')
	foreach service : ['passwd', 'group', 'shadow', 'hosts']
		name = '-'.join(['nss', service, 'test'])
		test(
			name,
			executable(
				name,
				sources: ['nss_' + service + '_test.cc'] + override_nss_src,
				include_directories: src,
				dependencies: [unistdx, gtest, ggg_core],
				link_with: [libnss_ggg],
				cpp_args: ['-DGGG_NSS_SERVICE="' + service + '"'],
			),
			suite: 'unit',
			is_parallel: false
		)
	endforeach
endif

if with_pam
	pam_wrapper = dependency('pam_wrapper', required: false)
	if pam_wrapper.found() and not state_dir.startswith('/') and b_sanitize != 'address'
		ld_preload_items = ['libpam_wrapper.so']
		ld_preload = 'LD_PRELOAD=' + ' '.join(ld_preload_items)
		test(
			'pam-test',
			executable(
				'pam-test',
				sources: ['pam_test.cc'],
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
			suite: 'unit',
			is_parallel: false
		)
		subdir('pam.d')
	endif
endif

libdl = cpp.find_library('dl', required: false)
if libdl.found()
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
	test(
		'cli-test',
		executable(
			'cli-test',
			sources: ['cli_test.cc'] + test_src,
			dependencies: [unistdx, gtest_nomain, pam, ggg_crypt, libsodium, ggg_core],
            link_with: override_fs,
		),
		env: [
            'LD_PRELOAD=' + override_fs.full_path(),
			'GGG_EXECUTABLE=' + ggg_exe.full_path(),
			'GUILE_AUTO_COMPILE=0'
		],
		suite: 'unit',
		timeout: 120,
        workdir: meson.build_root()
	)
endif
