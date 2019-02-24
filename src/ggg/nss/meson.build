nss_src = files([
	'ethers.cc',
	'group.cc',
	'hosts.cc',
	'passwd.cc',
	'shadow.cc',
])

libnss_ggg = shared_library(
	'nss_' + module_name,
	sources: nss_src,
	include_directories: inc,
	dependencies: [unistdx,ggg_core],
	version: '2',
	install: true
)
