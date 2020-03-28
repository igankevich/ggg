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

ggg_exe = executable(
    executable_name,
    sources: ggg_src,
    include_directories: src,
    dependencies: [unistdx,libsodium,ggg_core,guile,ggg_guile],
    install: true
)

ggg_public_keys_exe = executable(
    'ggg-public-keys',
    sources: ['ggg_public_keys_main.cc', 'cli_traits.cc'],
    include_directories: src,
    dependencies: [unistdx,libsodium,ggg_core,guile,ggg_guile],
    install: true
)

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

if has_override_fs and with_tests
    test(
        'cli-test',
        executable(
            'cli-test',
            sources: ['cli_test.cc'],
            dependencies: [unistdx, gtest_nomain, pam, libsodium, ggg_core, ggg_test],
        ),
        env: [
            'LD_PRELOAD=' + override_fs.full_path(),
            'GGG_EXECUTABLE=' + ggg_exe.full_path(),
            'GUILE_AUTO_COMPILE=0',
            'GUILE_LOAD_PATH=' + guile_load_path,
            'GGG_TEST_SUFFIX=cli-test'
        ],
        timeout: 120,
        workdir: meson.build_root(),
    )
endif
