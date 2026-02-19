#ifndef __CONFIG_H__
#define __CONFIG_H__

/* GLFW */
#define GLFW_WINDOW_WIDTH 1200
#define GLFW_WINDOW_HEIGHT 900

/* Filepaths */
// TODO: externalize the shader path at build time
// There's probably a library for this shit or something
#define CONFIG_SHADER_PATH "src/shaders/"
#define CONFIG_SHADER_VERTEX "vertex.glsl"
#define CONFIG_SHADER_FRAGMENT "fragment.glsl"

/* Audio Settings */
#define AUDIO_SAMPLE_RATE 44100
#define AUDIO_BUFFER_SIZE 2048
#define AUDIO_FFT_SIZE 2048
#define AUDIO_SMOOTHING 0.9f

/* PulseAudio source - set to your source or NULL for default mic.
 * Run 'pactl list sources short' to find available sources.
 * For microphone: use NULL (default input) or a specific mic source
 * For system audio: use a .monitor source like "alsa_output.xxx.monitor" */
#define AUDIO_PULSE_SOURCE NULL

#endif
