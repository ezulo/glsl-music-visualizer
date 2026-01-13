#ifndef __AUDIO_H__
#define __AUDIO_H__

#include <stddef.h>

/*
 * Initialize audio capture and FFT processing.
 * Returns 0 on success, -1 on failure.
 */
int audio_init(void);

/*
 * Cleanup audio resources.
 */
void audio_cleanup(void);

/*
 * Process audio and update FFT data.
 * Call this each frame before rendering.
 */
void audio_update(void);

/*
 * Get the FFT magnitude data.
 * Returns a pointer to FFT_SIZE/2 floats (normalized 0.0-1.0).
 */
const float *audio_get_fft_data(void);

/*
 * Get the number of FFT bins available.
 */
size_t audio_get_fft_size(void);

#endif
