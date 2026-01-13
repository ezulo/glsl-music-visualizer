#include "audio.h"
#include "config.h"

#include <fftw3.h>
#include <math.h>
#include <pthread.h>
#include <pulse/error.h>
#include <pulse/simple.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Audio capture state */
static pa_simple *pa_handle = NULL;
static pthread_t audio_thread;
static volatile int audio_running = 0;

/* Audio buffers */
static float *sample_buffer = NULL;
static pthread_mutex_t buffer_mutex = PTHREAD_MUTEX_INITIALIZER;

/* FFT state */
static double *fft_input = NULL;
static fftw_complex *fft_output = NULL;
static fftw_plan fft_plan = NULL;
static float *fft_magnitudes = NULL;

/*
 * Audio capture thread - continuously reads from PulseAudio
 */
static void *audio_thread_func(void *arg) {
  (void)arg;

  int16_t raw_buffer[AUDIO_BUFFER_SIZE];
  int error;

  while (audio_running) {
    /* Read audio samples from PulseAudio */
    if (pa_simple_read(pa_handle, raw_buffer, sizeof(raw_buffer), &error) < 0) {
      fprintf(stderr, "PulseAudio read error: %s\n", pa_strerror(error));
      continue;
    }

    /* Convert to float and copy to shared buffer */
    pthread_mutex_lock(&buffer_mutex);
    for (int i = 0; i < AUDIO_BUFFER_SIZE; i++) {
      sample_buffer[i] = raw_buffer[i] / 32768.0f;
    }
    pthread_mutex_unlock(&buffer_mutex);
  }

  return NULL;
}

int audio_init(void) {
  int error;

  /* PulseAudio sample spec */
  pa_sample_spec ss = {
      .format = PA_SAMPLE_S16LE,
      .rate = AUDIO_SAMPLE_RATE,
      .channels = 1,
  };

  /* Buffer attributes for low-latency capture */
  pa_buffer_attr buffer_attr = {
      .maxlength = (uint32_t)-1,
      .tlength = (uint32_t)-1,
      .prebuf = (uint32_t)-1,
      .minreq = (uint32_t)-1,
      .fragsize = AUDIO_BUFFER_SIZE * sizeof(int16_t),
  };

  /* Connect to PulseAudio - use default monitor source
   * You can specify a specific source like "alsa_output.pci-xxx.monitor"
   * or pass NULL to use the default source */
  pa_handle = pa_simple_new(NULL,              /* Server (NULL = default) */
                            "GLSL Visualizer", /* Application name */
                            PA_STREAM_RECORD,  /* Stream direction */
                            AUDIO_PULSE_SOURCE, /* Source (NULL = default) */
                            "Audio FFT",       /* Stream description */
                            &ss,               /* Sample spec */
                            NULL,              /* Channel map (NULL = default) */
                            &buffer_attr,      /* Buffering attributes */
                            &error);

  if (!pa_handle) {
    fprintf(stderr, "PulseAudio connection failed: %s\n", pa_strerror(error));
    fprintf(stderr, "Hint: Set AUDIO_PULSE_SOURCE in config.h to your monitor "
                    "source.\n");
    fprintf(stderr, "      Run 'pactl list sources short' to see available "
                    "sources.\n");
    return -1;
  }

  /* Allocate buffers */
  sample_buffer = calloc(AUDIO_BUFFER_SIZE, sizeof(float));
  fft_input = fftw_malloc(sizeof(double) * AUDIO_FFT_SIZE);
  fft_output = fftw_malloc(sizeof(fftw_complex) * (AUDIO_FFT_SIZE / 2 + 1));
  fft_magnitudes = calloc(AUDIO_FFT_SIZE / 2, sizeof(float));

  if (!sample_buffer || !fft_input || !fft_output || !fft_magnitudes) {
    fprintf(stderr, "Failed to allocate audio buffers\n");
    audio_cleanup();
    return -1;
  }

  /* Create FFT plan */
  fft_plan = fftw_plan_dft_r2c_1d(AUDIO_FFT_SIZE, fft_input, fft_output,
                                  FFTW_MEASURE);

  if (!fft_plan) {
    fprintf(stderr, "Failed to create FFT plan\n");
    audio_cleanup();
    return -1;
  }

  /* Start audio capture thread */
  audio_running = 1;
  if (pthread_create(&audio_thread, NULL, audio_thread_func, NULL) != 0) {
    fprintf(stderr, "Failed to create audio thread\n");
    audio_running = 0;
    audio_cleanup();
    return -1;
  }

  printf("Audio initialized: %d Hz, %d samples, %d FFT bins\n", AUDIO_SAMPLE_RATE,
         AUDIO_BUFFER_SIZE, AUDIO_FFT_SIZE / 2);

  return 0;
}

void audio_cleanup(void) {
  /* Stop audio thread */
  if (audio_running) {
    audio_running = 0;
    pthread_join(audio_thread, NULL);
  }

  /* Free FFT resources */
  if (fft_plan) {
    fftw_destroy_plan(fft_plan);
    fft_plan = NULL;
  }
  if (fft_input) {
    fftw_free(fft_input);
    fft_input = NULL;
  }
  if (fft_output) {
    fftw_free(fft_output);
    fft_output = NULL;
  }
  if (fft_magnitudes) {
    free(fft_magnitudes);
    fft_magnitudes = NULL;
  }
  if (sample_buffer) {
    free(sample_buffer);
    sample_buffer = NULL;
  }

  /* Close PulseAudio */
  if (pa_handle) {
    pa_simple_free(pa_handle);
    pa_handle = NULL;
  }
}

void audio_update(void) {
  /* Copy samples to FFT input with Hann window */
  pthread_mutex_lock(&buffer_mutex);
  for (int i = 0; i < AUDIO_FFT_SIZE; i++) {
    /* Hann window to reduce spectral leakage */
    double window = 0.5 * (1.0 - cos(2.0 * M_PI * i / (AUDIO_FFT_SIZE - 1)));
    int sample_idx = i % AUDIO_BUFFER_SIZE;
    fft_input[i] = sample_buffer[sample_idx] * window;
  }
  pthread_mutex_unlock(&buffer_mutex);

  /* Execute FFT */
  fftw_execute(fft_plan);

  /* Calculate magnitudes and normalize */
  for (int i = 0; i < AUDIO_FFT_SIZE / 2; i++) {
    double real = fft_output[i][0];
    double imag = fft_output[i][1];
    double magnitude = sqrt(real * real + imag * imag);

    /* Convert to decibels and normalize to 0-1 range */
    double db = 20.0 * log10(magnitude + 1e-10);
    double normalized = (db + 80.0) / 80.0; /* -80dB to 0dB range */

    /* Clamp to 0-1 */
    if (normalized < 0.0)
      normalized = 0.0;
    if (normalized > 1.0)
      normalized = 1.0;

    /* Simple smoothing */
    fft_magnitudes[i] =
        fft_magnitudes[i] * AUDIO_SMOOTHING + normalized * (1.0 - AUDIO_SMOOTHING);
  }
}

const float *audio_get_fft_data(void) { return fft_magnitudes; }

size_t audio_get_fft_size(void) { return AUDIO_FFT_SIZE / 2; }
