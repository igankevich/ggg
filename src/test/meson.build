test(
	'entity-test',
	executable(
		'entity-test',
		sources: ['entity_test.cc'],
		include_directories: inc,
		dependencies: [unistdx, gtest, ggg_core]
	)
)

test(
	'group-test',
	executable(
		'group-test',
		sources: ['group_test.cc'],
		include_directories: inc,
		dependencies: [unistdx, gtest, ggg_core]
	)
)

test(
	'account-test',
	executable(
		'account-test',
		sources: ['account_test.cc'],
		include_directories: inc,
		dependencies: [unistdx, gtest, ggg_core]
	)
)

test(
	'hierarchy-test',
	executable(
		'hierarchy-test',
		sources: ['hierarchy_test.cc'],
		include_directories: inc,
		dependencies: [unistdx, gtest, ggg_core]
	)
)

if etc.startswith('/')

	ld_preload = ''
	if get_option('sanitize') == 'address'
		ld_preload += 'LD_PRELOAD=libasan.so.3 libnss_wrapper.so'
	elif get_option('sanitize') == 'none'
		ld_preload += 'LD_PRELOAD=libnss_wrapper.so'
	endif
	
	test(
		'pwd-test',
		executable(
			'pwd-test',
			sources: ['pwd_test.cc'],
			include_directories: inc,
			dependencies: [unistdx, gtest, ggg_core],
		),
		env: [
			ld_preload,
			'NSS_WRAPPER_MODULE_SO_PATH=' + libnss_hierarchy.full_path(),
			'NSS_WRAPPER_MODULE_FN_PREFIX=' + module_name,
			'NSS_WRAPPER_DEBUGLEVEL=3',
			'NSS_WRAPPER_PASSWD=passwd',
			'NSS_WRAPPER_GROUP=group',
		]
	)
	
	test(
		'grp-test',
		executable(
			'grp-test',
			sources: ['grp_test.cc'],
			include_directories: inc,
			dependencies: [unistdx, gtest, ggg_core],
		),
		env: [
			ld_preload,
			'NSS_WRAPPER_MODULE_SO_PATH=' + libnss_hierarchy.full_path(),
			'NSS_WRAPPER_MODULE_FN_PREFIX=' + module_name,
			'NSS_WRAPPER_DEBUGLEVEL=3',
			'NSS_WRAPPER_PASSWD=passwd',
			'NSS_WRAPPER_GROUP=group',
		]
	)

endif

test(
	'conversation-test',
	executable(
		'conversation-test',
		sources: ['conversation_test.cc'],
		include_directories: [inc, pam_inc],
		link_with: pam_ggg,
		dependencies: [unistdx, gtest, libcrypt, pam, ggg_core]
	)
)
test(
	'form-field-test',
	executable(
		'form-field-test',
		sources: ['form_field_test.cc'],
		include_directories: inc,
		link_with: pam_ggg,
		dependencies: [unistdx, gtest, pam, libcrypt]
	)
)

