#version 330 core

#define FREQ_MIN 0.2f
#define FREQ_MAX 0.7f

out vec4 FragColor;

uniform float u_time;
uniform vec2 u_resolution;
uniform sampler1D u_fft;

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

void main() {
    vec2 uv;
    vec3 color, bg_color, final_color;
    float dist;

    // Sample audio frequencies
    float bass = get_bass();
    float mid = get_mid();
    float treble = get_treble();


    // Normalize coordinates to [-1, 1] with aspect ratio correction
    uv = (gl_FragCoord.xy * 2.0 - u_resolution) / min(u_resolution.x, u_resolution.y);
    dist = length(uv);

    // Normalize x to exactly [-1, 1] for bar calculation (no aspect ratio correction)
    float bar_x = (gl_FragCoord.x / u_resolution.x) * 2.0 - 1.0;

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

    // Normalize y to [0, 1] for bar height comparison (0 = bottom, 1 = top)
    float bar_y = gl_FragCoord.y / u_resolution.y;

    // Bar height is determined by the frequency amplitude
    float bar_height = freq_amplitude;

    // Set bar color (brighter at top)
    color = vec3(1.0f, bar_y, 0.0f);

    // Background color
    bg_color = vec3(0.1f, 0.1f, 0.15f);

    // Draw bar if y position is below the bar height and not in a gap
    if (bar_y < bar_height && !in_gap) {
        final_color = color;
    } else {
        final_color = bg_color;
    }

    FragColor = vec4(final_color, 1.0f);
}
