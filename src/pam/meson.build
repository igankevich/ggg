conversation_src = files([
	'conversation.cc'
])

pam_src = files([
	'pam_ggg.cc',
	'pam_handle.cc',
	'pam_category.cc',
]) + conversation_src

shared_library(
	'pam_' + module_name,
	sources: pam_src + hierarchy_src,
	include_directories: inc,
	dependencies: [pam,unistdx,libcrypt]
)


