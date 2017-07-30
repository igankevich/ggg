ggg_src = files([
	'ggg_main.cc',
	'command.cc'
])

executable(
	executable_name,
	sources: ggg_src + hierarchy_src,
	include_directories: inc,
	dependencies: [unistdx,libcrypt]
)
