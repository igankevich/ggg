csd = meson.current_source_dir()
cbd = meson.current_build_dir()

# main.xml {{{
main_xml = configuration_data()
main_xml.set('version', meson.project_version())
main_xml.set('src', csd)
configure_file(
	input: 'main.xml',
	output: 'main.xml',
	configuration: main_xml
)
# }}}

# validate {{{
xmllint = find_program('xmllint', required: false)
if xmllint.found()
	run_target(
		'man-validate',
		command: [
			xmllint,
			'--noout',
			'--xinclude',
			'--relaxng',
			join_paths(csd, 'docbookxi.rng'),
			join_paths(cbd, 'main.xml'),
		]
	)
endif
# }}}

# man pages {{{
mandir = join_paths(get_option('prefix'), get_option('mandir'))
xsltproc = find_program('xsltproc')
inputs = [
	'ggg.xml',
	'main.xml'
]
outputs = [
	'ggg.1'
]
output_dirs = []
foreach filename : outputs
	section = 'man' + filename.split('.')[1]
	output_dirs += join_paths(mandir, section)
endforeach
if xsltproc.found()
	man = custom_target(
		'man',
		command: [
			xsltproc,
			'--nonet',
			'--novalid',
			'--xinclude',
			'--output',
			cbd + '/',
			join_paths(csd, 'man.xsl'),
			join_paths(cbd, 'main.xml'),
		],
		input: inputs,
		output: outputs,
		build_by_default: true,
		install: true,
		install_dir: output_dirs
	)
endif
# }}}
