nss_src = files([
	'pwd.cc',
	'grp.cc',
	'shadow.cc'
])

libnss_hierarchy = shared_library(
	'nss_' + module_name,
	sources: hierarchy_src + nss_src,
	include_directories: inc,
	dependencies: [unistdx],
	version: '2',
	install: true
)


