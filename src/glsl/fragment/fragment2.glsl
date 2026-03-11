#version 330 core

#define FREQ_MIN 0.2f
#define FREQ_MAX 0.7f
#define PI 3.14159

out vec3 FragColor;

uniform float u_time;
uniform vec2 u_resolution;
uniform sampler1D u_fft;
uniform float u_tracer_mult;
uniform sampler2D u_prev_frame;

/*
 * Sample FFT at a normalized frequency (0.0 = lowest, 1.0 = highest).
 * The FFT data is stored in a 1D texture with magnitudes normalized to 0-1.
 */
float get_freq(float freq_normalized) {
  return texture(u_fft, freq_normalized).r;
}

/*
 * Get bass (low frequencies), mid, and treble (high frequencies).
 * These sample different regions of the FFT spectrum.
 */
float get_bass()   { return get_freq(0.05); }
float get_mid()    { return get_freq(0.2); }
float get_treble() { return get_freq(0.5); }

/*
 * cosine-based palette function https://iquilezles.org/articles/palettes/
 */
vec3 palette(in float t) {
  vec3 a = vec3(0.41f, 0.40f, 0.72f);  // center: midpoint between sky blue and indigo
  vec3 b = vec3(0.12f, 0.40f, 0.20f);  // amplitude: oscillation range
  vec3 c = vec3(0.5f, 0.5f, 0.5f);     // frequency of color change
  vec3 d = vec3(0.0f, 0.0f, 0.0f);     // phase offset
  return a + b * cos( PI * 2.0f * (c * t + d) );
}

void main() {
  vec2 uv;
  vec2 fbo_offset;
  vec3 color, bg_color, final_color;
  float dist, bar_x, bar_y;

  // Sample audio frequencies
  float bass = get_bass();
  float mid = get_mid();
  float treble = get_treble();

  // Sample previous frame
  // Normalize coordinates to [-1, 1] with aspect ratio correction
  //uv = (gl_FragCoord.xy * 2.0 - u_resolution) / min(u_resolution.x, u_resolution.y);
  uv = gl_FragCoord.xy / u_resolution * 2.0 - 1.0;
  dist = length(uv);

  // Tracer offset
  fbo_offset = vec2(0.0f, 0.0f) / u_resolution;

  vec3 prev = texture(u_prev_frame, fbo_offset + gl_FragCoord.xy / u_resolution).rgb;
  prev *= float(u_tracer_mult); // darken

  // Normalize x to exactly [-1, 1] for bar calculation (no aspect ratio correction)
  bar_x = uv.x;

  // Number of bars on the screen
  int num_bars = 40;
  float interval = 1.0f / (float(num_bars) / 2.0f);

  // Position within the current bar (0.0 to 1.0)
  float bar_local = fract(bar_x / interval);

  // Gap size (0.0 = no gap, 0.5 = half the bar is gap)
  float gap = 0.1f;
  bool in_gap = bar_local < gap || bar_local > (1.0f - gap);

  // Get the frequency range from -1.0 to 1.0
  float freq = interval * floor(bar_x / interval) + (interval / 2.0f);

  // Normalize to 0.0 to 1.0
  freq = freq / 2.0f;
  freq = freq + 0.5f;

  // Get amplitude for this bar's frequency range
  float freq_amplitude = get_freq(mix(FREQ_MIN, FREQ_MAX, freq));
  freq_amplitude = pow(freq_amplitude, 2);

  // Compute y value
  /* bottom */
  // float bar_y = (uv.y + 1.0) * 0.5f;
  /* centered (along x axis) */
  bar_y = abs(uv.y);

  // Bar height is determined by the frequency amplitude
  float bar_height = freq_amplitude;

  // Set bar color 
  color = palette(uv.y);

  // Background color
  bg_color = vec3(0.1f, 0.1f, 0.15f);

  // Draw bar if y position is below the bar height and not in a gap
  if (bar_y < bar_height && !in_gap) {
    final_color = color;
  } else {
    final_color = bg_color;
  }

  FragColor = max(final_color, prev);
}
