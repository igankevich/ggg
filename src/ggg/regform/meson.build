gtk3 = dependency('gtk+-3.0')
threads = dependency('threads')

regform_cpp_args = ['-DGTK_DISABLE_DEPRECATED']
if cpp.has_argument('-Wno-parentheses')
	regform_cpp_args += '-Wno-parentheses'
endif

executable(
	'ggg-regform',
	sources: ggg_pam_src + forms_src + ['regform.cc'],
	include_directories: src,
	dependencies: [pam, gtk3, ggg_crypt, threads, unistdx, ggg_core, zxcvbn],
	cpp_args: regform_cpp_args,
	install: true
)

