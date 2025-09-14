//  titania project
//  https://git.sr.ht/~chronovore/titania
//  SPDX-License-Identifier: MPL-2.0

#include <CoreAudio/AudioHardware.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/hid/IOHIDKeys.h>
#include <IOKit/hid/IOHIDManager.h>

#include "structures.h"
#include "titania.h"

// in theory, we can find by uid "AppleUSBAudioEngine:Sony Interactive Entertainment:DualSense (Edge)? Wireless Controller:{location id}:1"
titania_error find_audio_from_location(const uint32_t location_id, AudioDeviceID* audio_device) {
	AudioObjectPropertyAddress address = {
		kAudioHardwarePropertyDevices,
		kAudioObjectPropertyScopeGlobal,
		kAudioObjectPropertyElementMain
	};

	*audio_device = 0;

	uint32_t data_size = 0;
	OSStatus status = AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &address, 0, nullptr, &data_size);
	if (status != noErr) {
		return TITANIA_ERROR_HAPTICS_INVALID_HANDLE;
	}

	const uint32_t number_of_devices = data_size / sizeof(AudioDeviceID);
	if (number_of_devices > 32) {
		return TITANIA_ERROR_HAPTICS_INVALID_HANDLE;
	}
	AudioDeviceID audio_devices[32];

	status = AudioObjectGetPropertyData(kAudioObjectSystemObject, &address, 0, nullptr, &data_size, audio_devices);
	if (status != noErr) {
		return TITANIA_ERROR_HAPTICS_INVALID_HANDLE;
	}

	for (uint32_t i = 0; i < number_of_devices; i++) {
		const AudioDeviceID current = audio_devices[i];

		uint32_t transport_type = 0;
		data_size = sizeof(uint32_t);
		address = (AudioObjectPropertyAddress) {
			kAudioDevicePropertyTransportType,
			kAudioObjectPropertyScopeGlobal,
			kAudioObjectPropertyElementMain
		};

		status = AudioObjectGetPropertyData(current, &address, 0, nullptr, &data_size, &transport_type);
		if (status != noErr || transport_type != kAudioDeviceTransportTypeUSB) {
			continue;
		}

		CFStringRef uid = nullptr;
		data_size = sizeof(CFStringRef);
		address = (AudioObjectPropertyAddress) {
			kAudioDevicePropertyDeviceUID,
			kAudioObjectPropertyScopeGlobal,
			kAudioObjectPropertyElementMain
		};

		status = AudioObjectGetPropertyData(current, &address, 0, nullptr, &data_size, &uid);
		if (status != noErr || uid == nullptr) {
			continue;
		}

		char uid_str[256] = { 0 };
		const bool success = CFStringGetCString(uid, uid_str, sizeof(uid_str), kCFStringEncodingASCII);
		CFRelease(uid);
		if (!success) {
			continue;
		}

		char* last_colon = strrchr(uid_str, ':');
		if (last_colon == nullptr) {
			continue;
		}
		*last_colon = 0;

		last_colon = strrchr(uid_str, ':');
		if (last_colon == nullptr) {
			continue;
		}

		const uint32_t audio_location_id = (uint32_t) strtoul(last_colon + 1, nullptr, 16);
		if (audio_location_id != location_id) {
			continue;
		}

		address = (AudioObjectPropertyAddress) {
			kAudioDevicePropertyStreamFormat,
			kAudioDevicePropertyScopeOutput,
			kAudioObjectPropertyElementMain
		};

		AudioStreamBasicDescription stream_desc = {0};
		data_size = sizeof(AudioStreamBasicDescription);
		status = AudioObjectGetPropertyData(current, &address, 0, nullptr, &data_size, &stream_desc);
		if (status != noErr ||
			stream_desc.mSampleRate != 48000 ||
			stream_desc.mChannelsPerFrame != 4 ||
			stream_desc.mBitsPerChannel != 32 ||
			stream_desc.mFormatFlags != (kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked) ||
			stream_desc.mFormatID != kAudioFormatLinearPCM) {
			continue;
		}

		*audio_device = current;
		return TITANIA_ERROR_OK;
	}

	return TITANIA_ERROR_HAPTICS_INVALID_HANDLE;
}

titania_error feed_coreaudio_inner(titania_handle handle, AudioBufferList *output_data) {
	if (output_data->mNumberBuffers != 1 || output_data->mBuffers[0].mNumberChannels != 4) {
		return TITANIA_ERROR_INVALID_DATA;
	}

	CHECK_INIT();
	CHECK_HANDLE(handle);

	const size_t write = state[handle].haptics.write_offset;
	const size_t read = state[handle].haptics.read_offset;
	if (read > write) {
		state[handle].haptics.read_offset = write;
		return TITANIA_ERROR_OK;
	}

	size_t remain = (TITANIA_MAXIMUM_HAPTICS_SIZE - ((write - read) % TITANIA_MAXIMUM_HAPTICS_SIZE)) >> 1;

	const size_t avail = output_data->mBuffers[0].mDataByteSize >> 2;
	if (remain > avail) {
		remain = avail;
	}

	if (remain < TITANIA_MINIMUM_HAPTICS_SIZE >> 1) {
		return TITANIA_ERROR_OK;
	}

	float *output = output_data->mBuffers[0].mData;
	for (size_t sample_idx = 0; sample_idx < remain; ++sample_idx) {
		output[(sample_idx << 2) + 2] = state[handle].haptics.buffer[sample_idx << 1];
		output[(sample_idx << 2) + 3] = state[handle].haptics.buffer[(sample_idx << 1) + 1];
	}

	state[handle].haptics.read_offset += (remain << 1);

	return TITANIA_ERROR_OK;
}

OSStatus feed_coreaudio(AudioDeviceID device, const AudioTimeStamp *now,
	const AudioBufferList *input_data, const AudioTimeStamp *input_time,
	AudioBufferList *output_data, const AudioTimeStamp *output_time,
	void* user_data) {
	titania_handle handle = (titania_handle) (intptr_t) user_data;
	if (IS_TITANIA_BAD(feed_coreaudio_inner(handle, output_data))) {
		return kAudioHardwareUnspecifiedError;
	}

	return noErr;
}

titania_error titania_haptics_init(const titania_handle handle) {
	CHECK_INIT();
	CHECK_HANDLE_VALID(handle);

	if (IS_ACCESS(state[handle].hid_info)) {
		return TITANIA_ERROR_OK;
	}

	if (state[handle].hid_info.is_bluetooth) {
		return titania_haptics_init_bt(handle);
	}

	const io_service_t service = IOHIDDeviceGetService(*(IOHIDDeviceRef*) state[handle].hid);
	const CFTypeRef locationIdRef = IORegistryEntrySearchCFProperty(service, kIOServicePlane, CFSTR("LocationID"), kCFAllocatorDefault, 0);
	titania_error result = TITANIA_ERROR_HAPTICS_INVALID_HANDLE;
	if (locationIdRef) {
		if (CFGetTypeID(locationIdRef) == CFNumberGetTypeID()) {
			uint32_t locationId;
			if (CFNumberGetValue(locationIdRef, kCFNumberSInt32Type, &locationId)) {
				result = find_audio_from_location(locationId, &state[handle].haptics.device_id);
				if (result == TITANIA_ERROR_OK) {
					AudioDeviceCreateIOProcID(state[handle].haptics.device_id, feed_coreaudio, (void*) (intptr_t) handle, &state[handle].haptics.proc_id);
					AudioDeviceStart(state[handle].haptics.device_id, state[handle].haptics.proc_id);
				}
			}
		}
	}
	CFRelease(locationIdRef);
	IOObjectRelease(service);

	return result;
}

titania_error titania_haptics_close(const titania_handle handle) {
	CHECK_INIT();
	CHECK_HANDLE_VALID(handle);

	if (IS_ACCESS(state[handle].hid_info)) {
		return TITANIA_ERROR_OK;
	}

	if (state[handle].hid_info.is_bluetooth) {
		return titania_haptics_close_bt(handle);
	}

	if (state[handle].haptics.device_id == 0 || state[handle].haptics.proc_id == 0) {
		return TITANIA_ERROR_OK;
	}

	AudioDeviceStop(state[handle].haptics.device_id, state[handle].haptics.proc_id);

	return TITANIA_ERROR_OK;
}

titania_error titania_haptics_flush(const titania_handle handle) {
	CHECK_INIT();
	CHECK_HANDLE_VALID(handle);

	if (IS_ACCESS(state[handle].hid_info)) {
		return TITANIA_ERROR_OK;
	}

	if (state[handle].hid_info.is_bluetooth) {
		return titania_haptics_close_bt(handle);
	}

	if (state[handle].haptics.device_id == 0 || state[handle].haptics.proc_id == 0) {
		return TITANIA_ERROR_OK;
	}

	AudioDeviceStop(state[handle].haptics.device_id, state[handle].haptics.proc_id);
	state[handle].haptics.read_offset = 0;
	memset(state[handle].haptics.buffer, 0, sizeof(state[handle].haptics.buffer));
	AudioDeviceStart(state[handle].haptics.device_id, state[handle].haptics.proc_id);

	return TITANIA_ERROR_OK;
}
