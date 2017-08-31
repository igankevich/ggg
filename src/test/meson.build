libnss_hierarchy_test = shared_library(
	'nss_' + module_name + '_test',
	sources: hierarchy_src + nss_src,
	include_directories: inc,
	dependencies: [unistdx],
	version: '2',
	cpp_args: ['-DGGG_TEST']
)

test(
	'entity-test',
	executable(
		'entity-test',
		sources: hierarchy_src + ['entity_test.cc'],
		include_directories: inc,
		dependencies: [unistdx, gtest]
	)
)

test(
	'group-test',
	executable(
		'group-test',
		sources: hierarchy_src + ['group_test.cc'],
		include_directories: inc,
		dependencies: [unistdx, gtest]
	)
)

test(
	'account-test',
	executable(
		'account-test',
		sources: hierarchy_src + ['account_test.cc'],
		include_directories: inc,
		dependencies: [unistdx, gtest]
	)
)

test(
	'hierarchy-test',
	executable(
		'hierarchy-test',
		sources: hierarchy_src + ['hierarchy_test.cc'],
		include_directories: inc,
		dependencies: [unistdx, gtest]
	)
)

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
		sources: hierarchy_src + ['pwd_test.cc'],
		include_directories: inc,
		dependencies: [unistdx, gtest],
		cpp_args: ['-DGGG_TEST']
	),
	env: [
		ld_preload,
		'NSS_WRAPPER_MODULE_SO_PATH=' + libnss_hierarchy_test.full_path(),
		'NSS_WRAPPER_MODULE_FN_PREFIX=' + module_name,
		'NSS_WRAPPER_DEBUGLEVEL=3',
		'NSS_WRAPPER_PASSWD=passwd',
		'NSS_WRAPPER_GROUP=group',
		'GGG_ENT_ROOT=h-pwd'
	]
)

test(
	'grp-test',
	executable(
		'grp-test',
		sources: hierarchy_src + ['grp_test.cc'],
		include_directories: inc,
		dependencies: [unistdx, gtest],
		cpp_args: ['-DGGG_TEST']
	),
	env: [
		ld_preload,
		'NSS_WRAPPER_MODULE_SO_PATH=' + libnss_hierarchy_test.full_path(),
		'NSS_WRAPPER_MODULE_FN_PREFIX=' + module_name,
		'NSS_WRAPPER_DEBUGLEVEL=3',
		'NSS_WRAPPER_PASSWD=passwd',
		'NSS_WRAPPER_GROUP=group',
		'GGG_ENT_ROOT=h-grp'
	]
)

test(
	'conversation-test',
	executable(
		'conversation-test',
		sources: pam_src + hierarchy_src + ['conversation_test.cc'],
		include_directories: [inc, pam_inc],
		dependencies: [unistdx, gtest, libcrypt, pam]
	)
)
test(
	'form-field-test',
	executable(
		'form-field-test',
		sources: pam_src + ['form_field_test.cc'],
		include_directories: inc,
		dependencies: [unistdx, gtest, pam, libcrypt]
	)
)

