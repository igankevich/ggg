types_scm_in = files(['types.scm.in'])
types_scm = configuration_data()
types_scm.set('prefix', join_paths(get_option('prefix'), get_option('libdir')))

configure_file(
    input: types_scm_in,
    output: 'types.scm',
    configuration: types_scm,
    install: true,
    install_dir: scm_directory
)

#guild_command = [
#    'env',
#    'LD_LIBRARY_PATH=' + guile_load_compiled_path,
#    'GUILE_LOAD_PATH=' + guile_load_path,
#    'GUILE_LOAD_COMPILED_PATH=' + guile_load_compiled_path,
#    'GUILE_AUTO_COMPILE=0',
#    find_program('guild').path(),
#    'compile',
#    '-o',
#    join_paths(meson.current_build_dir(), '@BASENAME@.go'),
#    '@INPUT@'
#]
#
#ggg_guile_types = custom_target(
#    'guile-types',
#    input: join_paths(meson.current_build_dir(), 'types.scm'),
#    output: 'types.go',
#    command: guild_command,
#    install: true,
#    install_dir: go_directory
#)
#
#ggg_guile_forms = custom_target(
#    'guile-forms',
#    input: 'forms.scm',
#    output: 'forms.go',
#    command: guild_command,
#    install: true,
#    install_dir: go_directory
#)

install_data(['forms.scm'], install_dir: scm_directory)
