pam_service = configuration_data()
pam_service.set('lib', ggg_pam_lib.full_path())
configure_file(
	input: 'pam_service.in',
	output: 'ggg',
	configuration: pam_service
)
