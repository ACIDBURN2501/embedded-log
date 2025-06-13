project(
  'embedded-log', 'c',
  version: '0.1.0',
  default_options: ['warning_level=2', 'c_std=c11']
)

embedded_log_inc = include_directories('include')

embedded_log_lib = static_library(
  'log',
  sources: ['src/log.c'],
  include_directories: embedded_log_inc,
)

install_headers(
  'include/log.h',
  subdir: ''
)

embedded_log_dep = declare_dependency(
  include_directories: embedded_log_inc,
  link_with: embedded_log_lib
)

if get_option('build_tests')
  subdir('test')
endif
