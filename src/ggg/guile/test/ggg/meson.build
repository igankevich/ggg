test_types_scm = configuration_data()
test_types_scm.set('prefix', guile_libdir)
configure_file(
    input: types_scm_in,
    output: 'types.scm',
    configuration: test_types_scm
)
