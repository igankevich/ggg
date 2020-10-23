ggg_src = files([
    'add_entity.cc',
    'all_commands.cc',
    'attach.cc',
    'backup.cc',
    'cli_traits.cc',
    'command.cc',
    'dot.cc',
    'edit_entity.cc',
    'editor.cc',
    'entity_type.cc',
    'expire_entity.cc',
    'expunge.cc',
    'ggg_main.cc',
    'guile.cc',
    'heal.cc',
    'lock_entity.cc',
    'messages.cc',
    'remove_entity.cc',
    'reset_password.cc',
    'search.cc',
    'show_duplicates.cc',
    'select.cc',
    'select_all.cc',
    'show_help.cc',
    'show_version.cc',
    'tmpfile.cc',
    'unlock_entity.cc',
])

foreach config : binary_configs
    tmp_ggg_exe = executable(
        config.get('prefix') + executable_name,
        sources: ggg_src,
        cpp_args: config.get('cpp_args'),
        include_directories: src,
        dependencies: [
            unistdx,
            libsodium,
            get_variable(config.get('prefix') + 'ggg_core'),
            guile,
            get_variable(config.get('prefix') + 'ggg_guile'),
        ],
        install: config.get('prefix') == ''
    )
    set_variable(config.get('prefix') + 'ggg_exe', tmp_ggg_exe)
endforeach

foreach config : binary_configs
    tmp_ggg_public_keys_exe = executable(
        config.get('prefix') + 'ggg-public-keys',
        sources: ['ggg_public_keys_main.cc', 'cli_traits.cc'],
        cpp_args: config.get('cpp_args'),
        include_directories: src,
        dependencies: [
            unistdx,
            libsodium,
            get_variable(config.get('prefix') + 'ggg_proto'),
            get_variable(config.get('prefix') + 'ggg_core'),
            get_variable(config.get('prefix') + 'ggg_guile'),
            guile,
        ],
        install: config.get('prefix') == ''
    )
    set_variable(config.get('prefix') + 'ggg_public_keys_exe', tmp_ggg_public_keys_exe)
endforeach

# guile environment variables {{{
guile_config = configuration_data()
guile_config.set('load_compiled_path', guile_load_compiled_path)
guile_config.set('load_path', guile_load_path)
configure_file(
    input: 'guile.in',
    output: 'guile',
    configuration: guile_config
)
# }}}

if with_integration_tests and with_tests
    test(
        'cli/ggg',
        executable(
            'cli-test',
            sources: ['cli_test.cc'],
            cpp_args: test_cpp_args,
            dependencies: [
                unistdx,
                gtest_nomain,
                pam,
                libsodium,
                test_ggg_core,
                ggg_test,
                ggg_proto_test
            ],
        ),
        env: ['GGG_PATH=' + ':'.join([meson.current_build_dir()])],
        timeout: 120,
        workdir: meson.build_root(),
    )
endif

if with_tests
    test(
        'cli/public_key',
        executable(
            'public-key-test',
            sources: ['public_key_test.cc'],
            cpp_args: test_cpp_args,
            dependencies: [unistdx, gtest, test_ggg_core, ggg_proto_test],
        ),
        workdir: meson.build_root(),
    )
endif
