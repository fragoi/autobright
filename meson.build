project('autobright', 'cpp',
  version: '0.1.0')

prefix = get_option('prefix')
datadir = prefix / get_option('datadir')
prvdir = prefix / 'lib' / meson.project_name()

headers = [
  'src/adapter.h',
  'src/autobright.h',
  'src/autobright-service.h',
  'src/brightness.h',
  'src/filter.h',
  'src/gdbus.h',
  'src/gexception.h',
  'src/gsettings.h',
  'src/idle-aware.h',
  'src/idle-monitor.h',
  'src/logger.h',
  'src/sensor.h',
  'src/settings.h',
]

sources = [
  'src/adapter.cpp',
  'src/autobright.cpp',
  'src/autobright-service.cpp',
  'src/brightness.cpp',
  'src/filter.cpp',
  'src/gdbus.cpp',
  'src/gexception.cpp',
  'src/gsettings.cpp',
  'src/idle-aware.cpp',
  'src/idle-monitor.cpp',
  'src/logger.cpp',
  'src/sensor.cpp',
  'src/settings.cpp',
]

dependencies = [
  dependency('gio-unix-2.0'),
  dependency('autobright-dbus',
    fallback: ['dbus', 'dep'])
]

tests = {
  'brightness_test': 'test/brightness_test.cpp',
  'closure_test': 'test/closure_test.cpp',
  'forward_test': 'test/forward_test.cpp',
  'gboxed_ptr_test': 'test/gboxed_ptr_test.cpp',
  'gexception_test': 'test/gexception_test.cpp',
  'gobject_ptr_test': 'test/gobject_ptr_test.cpp',
  'idle_aware_test': 'test/idle_aware_test.cpp',
  'idle_monitor_test': 'test/idle_monitor_test.cpp',
  'logger_test': 'test/logger_test.cpp',
  'promise_test': 'test/promise_test.cpp',
  'promisemm_test': 'test/promisemm_test.cpp',
  'retry_test': 'test/retry_test.cpp',
  'sensor_test': 'test/sensor_test.cpp',
  'signals_test': 'test/signals_test.cpp',
  'signals2_test': 'test/signals2_test.cpp',
  'splist_test': 'test/splist_test.cpp',
}

manual_test = {
  'brightness_manual_test': 'test/manual/brightness_test.cpp',
  'idle_aware_manual_test': 'test/manual/idle_aware_test.cpp',
  'idle_monitor_manual_test': 'test/manual/idle_monitor_test.cpp',
  'sensor_manual_test': 'test/manual/sensor_test.cpp',
}

lib = static_library(meson.project_name(),
  sources: headers + sources,
  dependencies: dependencies)

dep = declare_dependency(
  link_with: lib,
  dependencies: dependencies)

exe = executable(meson.project_name(),
  sources: 'src/main.cpp',
  dependencies: dep,
  install: true)

foreach name, source : tests
  test_exe = executable(name,
    sources: source,
    dependencies: dep,
    link_args: ['-lpthread'])
  test(name, test_exe)
endforeach

foreach name, source : manual_test
  executable(name,
    sources: source,
    dependencies: dep,
    link_args: ['-lpthread'])
endforeach

install_subdir('debugui',
  install_dir: prvdir
)

subdir('data' / 'desktop')
subdir('data' / 'schemas')
subdir('data' / 'systemd')

if get_option('debian')
  subdir('debian')
endif
