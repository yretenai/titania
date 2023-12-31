project(
	'libresense',
	'c',
	default_options : [
		'prefix=/usr',
		'c_std=c2x',
		'warning_level=3',
		'default_library=both'
	],
	license : 'MPL-2',
	meson_version : '>=1.2.0',
	version : '1.0.0'
)

compiler = meson.get_compiler('c')
linker = meson.get_compiler('c')

args = compiler.get_supported_arguments(
	'-fanalyzer',
	'-fms-extensions',
	'-Wall',
	'-Wextra',
	'-pedantic',
	'-Werror',
	'-Wno-unused-variable',
	'-Wno-unused-parameter',
	'-Wno-microsoft-fixed-enum'
)

if get_option('buildtype') == 'custom'
	args += compiler.get_supported_arguments(
		'-march=native',
		'-mtune=native'
	)
endif

configure_file(
	output : 'config.h',
	configuration : configuration_data({
										   'LIBRESENSE_MAX_CONTROLLERS' : get_option('libresense_max_controllers'),
										   'LIBRESENSE_PROJECT_NAME' : '"' + meson.project_name() + '"',
										   'LIBRESENSE_PROJECT_VERSION' : '"' + meson.project_version() + '"',
									   })
)

hidapi = dependency('hidapi', version : '>=0.13.0')
icu = dependency('icu-uc', version : '>=73.2')

libresense_inc = include_directories('include/')

if get_option('libresense_debug')
	add_project_arguments('-DLIBRESENSE_DEBUG', language : 'c')
endif

libresense_lib = library(meson.project_name(), [
							 'src/hid.c',
							 'src/enums.c',
							 'src/edge.c',
							 'src/trans.c',
							 'src/crc.c'
						 ],
						 dependencies : [hidapi, icu],
						 gnu_symbol_visibility: 'hidden',
						 c_args : [args, '-DLIBRESENSE_EXPORTING'],
						 install : true,
						 include_directories : libresense_inc
)

libresense_dep = declare_dependency(link_with : libresense_lib, include_directories : libresense_inc)

if get_option('libresense_log')
	executable('libresense-log', [
				   'src/cli.c'
			   ],
			   c_args : args,
			   dependencies : [libresense_dep, hidapi],
			   install : true)
endif

if get_option('libresense_ctl')
	executable('libresensectl', [
				   'src/ctl/main.c'
			   ],
			   c_args : args,
			   dependencies : [libresense_dep, hidapi],
			   install : true)
endif

install_headers('include/libresense.h', preserve_path : false)

if get_option('libresense_man')
	pandoc = find_program('pandoc', required : false)
	if not pandoc.found()
		warning('pandoc not found, building man pages will not be possible')
	else
		pandoc_commands = [pandoc, '-s', '-t', 'man', '-f', 'gfm+definition_lists', '-o']
		custom_target(
			'dualsense-hid.7',
			output : 'dualsense-hid.7',
			input : 'man/dualsense-hid.7.md',
			build_always_stale : true,
			install : true,
			install_dir : join_paths(get_option('mandir'), 'man7'),
			command : [pandoc_commands, '@OUTPUT@', '@INPUT@']
		)

		custom_target(
			'libresense.1',
			output : 'libresense.1',
			input : 'man/libresense.1.md',
			build_always_stale : true,
			install : true,
			install_dir : join_paths(get_option('mandir'), 'man1'),
			command : [pandoc_commands, '@OUTPUT@', '@INPUT@']
		)

		if get_option('libresense_log')
			custom_target(
				'libresense-log.1',
				output : 'libresense-log.1',
				input : 'man/libresense-log.1.md',
				build_always_stale : true,
				install : true,
				install_dir : join_paths(get_option('mandir'), 'man1'),
				command : [pandoc_commands, '@OUTPUT@', '@INPUT@']
			)
		endif

		if get_option('libresense_ctl')
			custom_target(
				'libresensectl.1',
				output : 'libresensectl.1',
				input : 'man/libresensectl.1.md',
				build_always_stale : true,
				install : true,
				install_dir : join_paths(get_option('mandir'), 'man1'),
				command : [pandoc_commands, '@OUTPUT@', '@INPUT@']
			)
		endif
	endif
endif
