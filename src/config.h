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
#define AUDIO_SMOOTHING 0.8f

/* PulseAudio source - set to your monitor source or NULL for default.
 * Run 'pactl list sources short' to find available sources.
 * Example: "alsa_output.pci-0000_00_1f.3.analog-stereo.monitor" */
#define AUDIO_PULSE_SOURCE                                                     \
  "alsa_output.usb-C-Media_Electronics_Inc._USB_Audio_Device-00.analog-"       \
  "stereo.monitor"

#endif
