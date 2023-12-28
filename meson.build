project(
  'libresense',
  'c',
  default_options: [
    'prefix=/usr',
    'c_std=c17',
    'warning_level=3',
    'default_library=both'
  ],
  license: 'MPL-2',
  meson_version: '>=1.2.0',
  version: '0.0.1'
)

res_path = get_option('datadir') + '/libresense'

if host_machine.system() == 'windows'
  res_path = './res'
endif

compiler = meson.get_compiler('c')
linker = meson.get_compiler('c')

args = compiler.get_supported_arguments(
  '-fanalyzer',
  '-ffast-math'
)

if get_option('buildtype') == 'custom'
  args += compiler.get_supported_arguments(
    '-march=knl',
    '-mtune=knl'
  )
endif

configure_file(
  output: 'config.h',
  configuration: configuration_data({
    'LIBRESENSE_MAX_CONTROLLERS': get_option('max_controllers'),
    'LIBRESENSE_PROJECT_NAME': '"' + meson.project_name() + '"',
    'LIBRESENSE_PROJECT_VERSION': '"' + meson.project_version() + '"',
    'LIBRESENSE_DATA_DIR': '"' + res_path + '"'  
  })
)

hidapi = dependency('hidapi', version: '>=0.13.0')
threads = dependency('threads')

libresense_inc = include_directories('include/')

if get_option('debug_logic')
  add_project_arguments('-DLIBRESENSE_DEBUG', language: 'c')
endif

libresense_lib = library(meson.project_name(), [
    'src/hid.c',
    'src/enums.c',
    'src/edge.c',
    'src/trans.c',
    'src/crc.c'
  ],
  dependencies: [hidapi, threads],
  c_args: args,
  install: true,
  include_directories: libresense_inc
)

libresense_dep = declare_dependency(link_with: libresense_lib, include_directories: libresense_inc)

if get_option('build_test_executable')
  executable('libresense-log', [
      'src/cli.c'
    ],
    c_args: args,
    dependencies: [libresense_dep, hidapi],
    install: true)
endif

install_headers('include/libresense.h', preserve_path: false)
