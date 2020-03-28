(use-package-modules check crypto guile gcc commencement sqlite linux
 gettext pkg-config unistdx sqlitex zxcvbn-c ggg ninja build-tools pre-commit)

(packages->manifest
 (list
  linux-pam libsodium guile-2.2 zxcvbn-c unistdx sqlitex
  googletest pkg-config ninja meson gettext-minimal python-pre-commit
  (list gcc "lib") gcc-toolchain pam-wrapper))

