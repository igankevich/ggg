nss_src = files([
	'pwd.cc',
	'grp.cc',
	'shadow.cc'
])

libnss_hierarchy = shared_library(
	'nss_' + module_name,
	sources: nss_src,
	include_directories: inc,
	dependencies: [unistdx,ggg_core],
	version: '2',
	install: true
)


