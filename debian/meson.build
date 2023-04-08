## Package descriptor file
configure_file(
  input: 'control.in',
  output: 'control',
  configuration: {
    'package': meson.project_name(),
    'version': meson.project_version()
  },
  install: true,
  install_dir: '/DEBIAN'
)

## Post-install script
configure_file(
  input: 'compile-schemas.sh.in',
  output: 'postinst',
  configuration: {
    'schemas_dir': schemas_dir
  },
  install: true,
  install_dir: '/DEBIAN'
)

## Post-remove script
configure_file(
  input: 'compile-schemas.sh.in',
  output: 'postrm',
  configuration: {
    'schemas_dir': schemas_dir
  },
  install: true,
  install_dir: '/DEBIAN'
)

## Script to generate package
configure_file(
  input: 'build-deb.sh.in',
  output: 'build-deb.sh',
  configuration: {
    'build_dir': meson.build_root(),
    'deb_name': meson.project_name() + '-' + meson.project_version()
  }
)
