configure_file(
  input: 'autobright.desktop.in',
  output: 'autobright.desktop',
  configuration: {
    'exec': prvdir / 'debugui' / 'app.js',
    'icon': prvdir / 'icons' / 'icon.svg'
  },
  install: true,
  install_dir: datadir / 'applications'
)
