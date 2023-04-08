project('autobright-dbus', 'c',
  version: '0.1.0')

gnome = import('gnome')

dependencies = [
  dependency('gio-unix-2.0')
]

sources = gnome.gdbus_codegen('dbus',
  sources: 'interfaces/com.github.fragoi.autobright.Debug.xml',
  interface_prefix: 'com.github.fragoi.autobright.',
  namespace: 'DBus',
  docbook: 'dbus-doc')

lib = static_library(meson.project_name(),
  sources: sources,
  dependencies: dependencies)

dep = declare_dependency(
  link_with: lib,
  dependencies: dependencies)
