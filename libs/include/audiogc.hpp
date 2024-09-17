// AudiOGC v1.0.0
// By HTV04

/*
MIT License

Copyright (c) 2022 HTV04

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef _AUDIOGC_
#define _AUDIOGC_

#pragma once

#include <string>

#define AUDIOGC_VERSION_MAJOR 1
#define AUDIOGC_VERSION_MINOR 0
#define AUDIOGC_VERSION_PATCH 0
#define AUDIOGC_VERSION_STRING "1.0.0"

namespace audiogc {

enum class type : unsigned char {
	//callback,
	detect,
	flac,
	mp3,
	vorbis,
	//raw,
	wav
};

enum class mode : unsigned char {
	stream, // Read from file as needed (recommended for music files)
	store // Store file into memory (recommended for sound effects)
	//store_raw // Store into memory and decode to raw PCM data (recommended for *small* sound effects)
};

struct object_data_t {
	// General data
	unsigned int instances;
	type audio_type;
	bool in_memory;
	mode file_mode;

	// File data
	char *file_name;

	// Memory data
	void *audio_data;
	unsigned int audio_size;
};

// The variables below are used to store the current state of the player.
// They should not be modified directly, and reading directly is discouraged.
struct data_t {
	// Audio library
	void *audio_handle; // Can be various types depending on the audio type.

	// Buffers
	void *buffers[2];
	int current_buffer;

	// Playback constants
	unsigned int buffer_size; // Default is DSP_STREAMBUFFER_SIZE * 2
	int buffer_rate;
	unsigned char channels;
	double max_pitch;

	// Playback state
	bool looping = false;
	double pitch = 1.0;
	int volume = 255;
	bool playing = false;
	int position = 0;
	bool true_stopped = true; // For "thread stops"

	// AESND
	void *aesnd_pb; // Type: AESNDPB

	// Threads
	unsigned int message_queue;
	void *thread_stackbase;
	unsigned int thread;

	// Thread settings
	unsigned int thread_stack_size; // Default is 0x8000, 4x the default stack size
	unsigned char thread_priority; // Default is 64, same as the main thread
};

class player {
	private:
		object_data_t *object_data;

		// Internal functions
		void get_file_data();
		void init();

	public:
		data_t data; // Do not modify!

		// Constructors
		player(type audio_type, const std::string &file_name, mode file_mode = mode::stream); // New player from file name.
		player(type audio_type, const void *audio_data, unsigned int audio_size); // New player from memory.
		player(const player &other); // New player from existing player.

		// Playback functions
		void pause(); // Pauses playback.
		bool play(); // Starts playback. If the audio is already playing, nothing happens. Returns whether the audio successfully started playing.
		void stop(); // Stops playback and resets position to 0.

		// Playback state functions
		unsigned char get_channel_count(); // Returns the number of channels, 1 for mono, 2 for stereo.
		double get_pitch(); // Returns the current pitch.
		unsigned char get_volume(); // Returns the current volume.
		bool is_looping(); // Returns whether the audio is looping.
		bool is_playing(); // Returns true if the audio is playing or paused.
		void seek(int offset = 0); // Seeks to the specified offset in milliseconds.
		void set_looping(bool looping = false); // Sets whether the audio should loop.
		double set_pitch(double pitch = 1.0); // Sets the pitch of the audio (1 = 1x, 1.5 = 1.5x, 2 = 2x, etc.). Returns the actual pitch set, should it translate to higher than the max frequency (144 kHz).
		void set_volume(unsigned char volume = 255); // Sets the volume to the specified value (0-255).
		double tell(); // Returns the current position in seconds.

		// Special functions
		void change_settings(unsigned int buffer_size = 2304, unsigned int thread_stack_size = 0x8000, unsigned char thread_priority = 64); // Changes internal player settings, only use if necessary!

		// Destructor
		~player(); // Frees all resources used by the player.
};

} // audiogc

#endif // !_AUDIOGC_
