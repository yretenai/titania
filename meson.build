project('titania', 'c',
	license : 'MPL-2',
	meson_version : '>=1.3.0',
	version : '3.0.0',
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

if get_option('buildtype') == 'debug'
	args += compiler.get_supported_arguments('-Werror')
endif

has_nullptr = compiler.compiles('int main(void) { return nullptr == 0; }\n', name : 'has nullptr', args : args, werror : true)

pack_test = '''
#pragma pack(push, 1)

struct nya {
	char a;
	int b;
};
static_assert(sizeof(struct nya) == 5, "fail");

#pragma pack(pop)
'''

has_pack = compiler.compiles(pack_test, name : 'has pack', args : args, werror : true)

if not has_pack
	fallback_pack_test = '''
	struct __attribute__((__packed__)) nya {
		char a;
		int b;
	};
	static_assert(sizeof(struct nya) == 5, "fail");
	'''
	if not compiler.compiles(fallback_pack_test, name : 'has packed attribute', args : args, werror : true)
		error('compiler cannot create densely packed structs?')
	endif
endif

stdc_test = '''
#pragma STDC FP_CONTRACT ON
#pragma STDC CX_LIMITED_RANGE ON
'''

has_stdc_flags = compiler.compiles(stdc_test, name : 'has stdc flags', args : args, werror : true)

bitpacking_test = '''
#include <stdint.h>

#ifdef HAS_PACK
#define PACKED
#pragma pack(push, 1)
#else
#define PACKED __attribute__((__packed__))
#endif

typedef struct PACKED bitpacked {
	uint8_t a : 7;
	uint16_t x : 12;
	uint16_t y : 12;
} bitpacked;

#ifdef HAS_PACK
#pragma pack(pop)
#undef PACKED
#endif

uint16_t test(uint32_t value) {
	bitpacked vec = *(bitpacked*) &value;
	return vec.y;
}

int main(void) {
	int n = test(0xf | (0x123 << 7) | (0x7ff << 19)); // 0x3ff8918f
	return !(n == 0x7ff);
}
'''

has_normal_bitpacking = false
bitpacking_test_run = compiler.run(bitpacking_test, name : 'has variable type bitpacking', args : args + [has_pack ? '-DHAS_PACK' : ''])
has_normal_bitpacking = bitpacking_test_run.compiled() and bitpacking_test_run.returncode() == 0

has_audio_haptics = get_option('titania_haptics_audio').allowed()
need_audio_haptics = get_option('titania_haptics_audio').enabled()
has_haptics_bt = get_option('titania_haptics_bt').allowed()
has_haptics_pulse = false
has_haptics_wasapi = false
has_haptics_coreaudio = false

hidapi = dependency('hidapi', version : '>=0.13.0', required : false)
if not hidapi.found()
	hidapi = dependency('hidapi')
endif

titania_source = [
	'src/access.c',
	'src/crc.c',
	'src/enums.c',
	'src/edge.c',
	'src/hid.c',
	'src/trans.c',
	'src/unicode.c'
]

titania_deps = [
	hidapi
]

if has_haptics_bt
	has_haptics = true
	titania_source += ['src/haptics_bt.c']
endif

if has_audio_haptics
	has_haptics = true
	has_haptics_bt = false
	ole = compiler.find_library('ole32', required : false)
	pulse = dependency('libpulse', required : false)
	coreaudio = dependency('appleframeworks', modules : ['CoreFoundation', 'IOKit', 'CoreAudio', 'AudioUnit'], required : false)

	if ole.found()
		message('using wasapi for haptics')
		titania_source += ['src/haptics_wasapi.c']
		titania_deps += [ole]
		has_haptics_wasapi = true
	elif coreaudio.found()
		message('using coreaudio for haptics')
		titania_source += ['src/haptics_coreaudio.c']
		titania_deps += [coreaudio]
		has_haptics_coreaudio = true
	elif pulse.found()
		message('using pulseaudio for haptics')
		titania_source += ['src/haptics_pulse.c']
		titania_deps += [pulse]
	else
		if need_haptics
			error('haptics required but no libraries found')
		else
			warning('haptics requested but no libraries found')
			has_haptics_bt = get_option('titania_haptics_bt').allowed()
		endif
	endif
endif

configure_file(
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
		'TITANIA_HAS_STDC_FLAGS': has_stdc_flags,
		'TITANIA_HAS_VARIABLE_BITPACKING' : has_normal_bitpacking,
		'TITANIA_HAS_HAPTICS' : has_haptics,
		'TITANIA_HAS_HAPTICS_PULSE' : has_haptics_pulse,
		'TITANIA_HAS_HAPTICS_COREAUDIO' : has_haptics_coreaudio,
		'TITANIA_HAS_HAPTICS_WASAPI' : has_haptics_wasapi,
		'TITANIA_HAS_HAPTICS_FALLBACK' : has_haptics_bt,
   })
)

titania_inc = include_directories('include/')

titania_lib = library(meson.project_name(),
	titania_source,
	dependencies : titania_deps,
	gnu_symbol_visibility : 'hidden',
	c_args : [args, '-DTITANIA_EXPORTING'],
	install : true,
	soversion : meson.project_version(),
	include_directories : titania_inc
)

titania_dep = declare_dependency(link_with : titania_lib, include_directories : titania_inc)

if get_option('titania_ctl')
	json = dependency('json', version : '>=1.0')
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
	description : 'unofficial dualsense and playstation access library',
	url : 'https://git.sr.ht/~chronovore/titania',
	version : meson.project_version()
)
