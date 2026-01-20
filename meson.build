project('sv-dashboard-gtk', 'cpp',
  version: '0.2.0',
  default_options: ['warning_level=3', 'cpp_std=c++20'])

gtkmm = dependency('gtkmm-3.0', required: true)
fc    = dependency('fontconfig', required: true)

sources = files(
  'src/main.cpp',
  'src/MainApp.cpp',
  'src/MainWindow.cpp',
  'src/Desktop.cpp',
  'src/DesktopIcon.cpp',
  'src/FontRegistry.cpp'
)

executable('sv-dashboard',
  sources: sources,
  include_directories: include_directories('src'),
  dependencies: [gtkmm, fc],
  install: true)

# Install bundled fonts (script populates assets/fonts before build in CI)
install_subdir('assets/fonts',
  install_dir: join_paths(get_option('datadir'), 'sv-dashboard-gtk', 'fonts'),
  strip_directory: true)
