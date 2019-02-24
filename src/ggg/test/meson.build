gtest = dependency('gtest', main: true)
gtest_nomain = dependency('gtest', main: false)

test_src = files([
	'chroot_environment.cc',
	'execute_command.cc',
])

override_nss_src = files([
	'override_nss.cc'
])

test(
	'entity-test',
	executable(
		'entity-test',
		sources: ['entity_test.cc'],
		include_directories: src,
		dependencies: [unistdx, gtest, ggg_core]
	),
	suite: 'unit'
)

test(
	'group-test',
	executable(
		'group-test',
		sources: ['group_test.cc'],
		include_directories: src,
		dependencies: [unistdx, gtest, ggg_core]
	),
	suite: 'unit'
)

test(
	'account-test',
	executable(
		'account-test',
		sources: ['account_test.cc'],
		include_directories: src,
		dependencies: [unistdx, gtest, ggg_core]
	),
	suite: 'unit'
)

test(
	'conversation-test',
	executable(
		'conversation-test',
		sources: ['conversation_test.cc'],
		include_directories: src,
		dependencies: [unistdx, gtest, ggg_crypt, pam, ggg_core, ggg_pam]
	),
	suite: 'unit'
)

test(
	'form-field-test',
	executable(
		'form-field-test',
		sources: ['form_field_test.cc'],
		include_directories: src,
		dependencies: [unistdx, gtest, pam, ggg_crypt, ggg_core, ggg_pam]
	),
	suite: 'unit'
)

if with_nss and not etc.startswith('/')
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
	if pam_wrapper.found() and not etc.startswith('/') and b_sanitize != 'address'
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

if not etc.startswith('/')
	test(
		'cli-test',
		executable(
			'cli-test',
			sources: ['cli_test.cc'] + test_src,
			dependencies: [unistdx, gtest_nomain, pam, ggg_crypt, ggg_core]
		),
		env: [
			'GGG_EXECUTABLE=' + ggg_exe.full_path(),
			'GGG_UNPRIVILEDGED=1'
		],
		suite: 'unit',
		timeout: 120
	)
endif

test(
	'ggg-dm-test',
	executable(
		'ggg-dm-test',
		sources: ['ggg_dm_test.cc'] + test_src,
		dependencies: [unistdx, gtest_nomain, pam, ggg_crypt, ggg_core]
	),
	suite: 'gui',
	timeout: 600
)

test(
	'ggg-form-test',
	executable(
		'ggg-form-test',
		sources: ['ggg_form_test.cc'] + test_src,
		dependencies: [unistdx, gtest_nomain, pam, ggg_crypt, ggg_core]
	),
	suite: 'cli',
	timeout: 600
)
