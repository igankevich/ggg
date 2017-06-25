pam_src = files([
	'pam_ggg.cc'
])
libnss_hierarchy = shared_library(
	'pam_' + module_name,
	sources: pam_src + hierarchy_src,
	include_directories: inc,
	dependencies: [pam,unistdx,libcrypt]
)


