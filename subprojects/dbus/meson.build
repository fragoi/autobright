project('autobright-dbus', 'c',
  version: '0.1.0')

gnome = import('gnome')

dependencies = [
  dependency('gio-unix-2.0')
]

sources = gnome.gdbus_codegen('autobright-dbus',
  sources: 'com.github.fragoi.Autobright.xml',
  interface_prefix: 'com.github.fragoi.autobright.',
  namespace: 'Autobright')

lib = static_library(meson.project_name(),
  sources: sources,
  dependencies: dependencies)

dep = declare_dependency(
  link_with: lib,
  include_directories: '.',
  dependencies: dependencies)
