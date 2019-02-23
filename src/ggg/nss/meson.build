nss_src = files([
	'ether.cc',
	'pwd.cc',
	'grp.cc',
	'shadow.cc',
	'hierarchy_instance.cc'
])

libnss_ggg = shared_library(
	'nss_' + module_name,
	sources: nss_src,
	include_directories: inc,
	dependencies: [unistdx,ggg_core],
	version: '2',
	install: true
)


