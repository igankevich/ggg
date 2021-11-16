(use-package-modules check crypto guile gcc commencement sqlite linux
 gettext pkg-config unistdx sqlitex zxcvbn-c ggg ninja build-tools)

(packages->manifest
 (list
  linux-pam libsodium guile-3.0 zxcvbn-c unistdx sqlitex
  googletest pkg-config ninja meson gettext-minimal
  (@ (gnu packages python-xyz) python-pre-commit)
  (list gcc "lib") gcc-toolchain pam-wrapper))

