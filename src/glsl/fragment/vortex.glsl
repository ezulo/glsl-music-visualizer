#version 330 core

#define FREQ_MIN 0.4f
#define FREQ_MAX 0.7f
#define PI 3.14159
#define PI_HALF 1.57079

out vec3 FragColor;

uniform float u_time;
uniform vec2 u_resolution;
uniform sampler1D u_fft;
uniform float u_tracer_mult;
uniform sampler2D u_prev_frame;

const float n_vertices = 1000.0f;
const float thickness = 0.035f;

/*
 * Sample FFT at a normalized frequency (0.0 = lowest, 1.0 = highest).
 * The FFT data is stored in a 1D texture with magnitudes normalized to 0-1.
 */
float get_freq(float freq_normalized) {
  return texture(u_fft, freq_normalized).r;
}

/*
 * cosine-based palette function https://iquilezles.org/articles/palettes/
 */
vec3 palette(in float t) {
  vec3 a = vec3(0.6f, 0.2f, 0.05f);   // dark red/orange base
  vec3 b = vec3(0.4f, 0.3f, 0.1f);    // amplitude: red -> yellow range
  vec3 c = vec3(1.0f, 1.0f, 1.0f);    // frequency
  vec3 d = vec3(0.0f, 0.15f, 0.20f);  // phase: offset G and B for fire gradient
  return a + b * cos( PI * 2.0f * (c * t + d) );
}

void main() {
  vec2 uv;
  float dist, granularity, freq, freq_amplitude, theta;
  vec3 bg_color, color, final_color, prev;

  // Normalize coordinates to [-1, 1] with aspect ratio correction
  //uv = (gl_FragCoord.xy * 2.0 - u_resolution) / min(u_resolution.x, u_resolution.y);
  uv = gl_FragCoord.xy / u_resolution * 2.0 - 1.0;
  dist = length(uv);

  // angle with respect to the x axis (0 to 1)
  theta = (u_time * -0.1f) + ((atan(uv.y, uv.x) / PI) + 1) * 0.5f;

  // Sample closer to center (scale < 1) = radiate outward
  float radiate_scale = 0.99f;
  vec2 texCoord = (uv * radiate_scale) * 0.5 + 0.5;
  prev = texture(u_prev_frame, texCoord).rgb;
  prev *= float(u_tracer_mult); // darken

  // get our frequency
  granularity = 1.0f / n_vertices;
  freq = fract(round(theta / granularity) * granularity);

  // Set bar color 
  color = palette(theta * 2.0f + u_time);

  // Background color
  bg_color = vec3(0.1f, 0.1f, 0.15f);

  freq_amplitude = get_freq(mix(FREQ_MIN, FREQ_MAX, freq));
  if (dist < freq_amplitude && dist > freq_amplitude - thickness) {
    final_color = color;
  } else {
    final_color = bg_color;
  }

  FragColor = max(final_color, prev);

}
