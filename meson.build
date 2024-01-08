project('titania', 'c',
	license : 'MPL-2',
	meson_version : '>=1.3.0',
	version : '1.5.1',
	default_options : [
		'c_std=c2x',
		'warning_level=3',
		'default_library=both'
	]
)

compiler = meson.get_compiler('c')
linker = meson.get_compiler('c')

args = compiler.get_supported_arguments(
	'-fanalyzer',
	'-Wall',
	'-Wextra',
	'-pedantic',
	'-Werror',
	'-Wno-unused-variable',
	'-Wno-unused-parameter',
	'-fcx-limited-range',
	'-ffp-contract=on',
	'/wd4820'
)

if get_option('buildtype') == 'custom'
	args += compiler.get_supported_arguments(
		'-march=native',
		'-mtune=native'
	)
endif

has_nullptr = false
if compiler.compiles('int main(void) { return nullptr == 0; }\n', name : 'has nullptr', args : args, werror : true)
	has_nullptr = true
endif

pack_test = '''
#pragma pack(push, 1)

struct nya {
	char a;
	int b;
};
static_assert(sizeof(struct nya) == 5, "");

#pragma pack(pop)
'''

has_pack = false
if compiler.compiles(pack_test, name : 'has pack', args : args, werror : true)
	has_pack = true
endif

stdc_test = '''
#pragma STDC FP_CONTRACT ON
#pragma STDC CX_LIMITED_RANGE ON
'''

has_stdc_flags = false
if compiler.compiles(stdc_test, name : 'has stdc flags', args : args, werror : true)
	has_stdc_flags = true
endif

config_file = configure_file(
	output : 'titania_config.h',
	configuration : configuration_data({
		'TITANIA_MAX_CONTROLLERS' : get_option('titania_max_controllers'),
		'TITANIA_PROJECT_NAME' : '"' + meson.project_name() + '"',
		'TITANIA_PROJECT_VERSION' : '"' + meson.project_version() + '"'
	}),
	install : true,
	install_dir : get_option('includedir')
)

configure_file(
	output : 'titania_config_internal.h',
	configuration : configuration_data({
		'TITANIA_HAS_NULLPTR' : has_nullptr,
		'TITANIA_HAS_PACK' : has_pack,
		'TITANIA_HAS_STDC_FLAGS': has_stdc_flags
   })
)

hidapi = dependency('hidapi', version : '>=0.13.0', required : false)
if not hidapi.found()
	hidapi = dependency('hidapi')
endif

titania_inc = include_directories('include/')

titania_lib = library(meson.project_name(), [
		'src/access.c',
		'src/crc.c',
		'src/enums.c',
		'src/edge.c',
		'src/hid.c',
		'src/trans.c',
		'src/unicode.c'
	],
	dependencies : [hidapi],
	gnu_symbol_visibility: 'hidden',
	c_args : [args, '-DTITANIA_EXPORTING'],
	install : true,
	soversion : 1,
	include_directories : titania_inc
)

titania_dep = declare_dependency(link_with : titania_lib, include_directories : titania_inc)

if get_option('titania_ctl')
	json = dependency('libjson', version : '>=1.0')
	executable('titaniactl', [
			'src/ctl/main.c',
			'src/ctl/modes/access.c',
			'src/ctl/modes/bench.c',
			'src/ctl/modes/bt.c',
			'src/ctl/modes/dump.c',
			'src/ctl/modes/edge.c',
			'src/ctl/modes/led.c',
			'src/ctl/modes/profile.c',
			'src/ctl/modes/report.c',
			'src/ctl/modes/test.c'
		],
		c_args : [args],
		dependencies : [titania_dep, hidapi, json],
	install : true)
endif

install_headers(['include/titania.h'], preserve_path : false)

if get_option('titania_man')
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
			'titania.1',
			output : 'titania.1',
			input : 'man/titania.1.md',
			build_always_stale : true,
			install : true,
			install_dir : join_paths(get_option('mandir'), 'man1'),
			command : [pandoc_commands, '@OUTPUT@', '@INPUT@']
		)

		if get_option('titania_ctl')
			custom_target(
				'titaniactl.1',
				output : 'titaniactl.1',
				input : 'man/titaniactl.1.md',
				build_always_stale : true,
				install : true,
				install_dir : join_paths(get_option('mandir'), 'man1'),
				command : [pandoc_commands, '@OUTPUT@', '@INPUT@']
			)
		endif
	endif
endif

import('pkgconfig').generate(
	titania_lib,
	description: 'unofficial dualsense and access driver',
	url: 'https://nothg.chronovore.dev/library/titania/'
)
