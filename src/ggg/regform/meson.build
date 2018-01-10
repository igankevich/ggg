gtk3 = dependency('gtk+-3.0')

executable(
	'ggg-regform',
	sources: pam_src + ['regform.cc'],
	include_directories: inc,
	dependencies: [pam, gtk3, libcrypt, threads, unistdx, ggg_core, zxcvbn],
	install: true
)

