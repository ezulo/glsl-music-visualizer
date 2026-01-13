#version 330 core

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
    vec2 uv_global, uv_local;
    vec3 color, bg_color, final_color;
    float dist_global, dist_local, wave_amplitude;

    // Sample audio frequencies
    float bass = get_bass();
    float mid = get_mid();
    float treble = get_treble();

    // Normalize coordinates to [-1, 1] with aspect ratio correction
    uv_global = (gl_FragCoord.xy * 2.0 - u_resolution) / min(u_resolution.x, u_resolution.y);
    dist_global = length(uv_global);

    uv_local = uv_global;
    uv_local *= 2.0f;
    uv_local = fract(uv_local);
    uv_local -= 0.5;

    dist_local = length(uv_local);

    // Wave amplitude modulated by bass
    wave_amplitude = 0.5f + sin(dist_local * bass * 3.0f) / 2.0f;
    wave_amplitude = step(0.85f, wave_amplitude);

    color = vec3(mid, 0.0f, treble);

    bg_color = vec3(0.1, 0.1, 0.15);

    // Mix circle and background
    final_color = mix(bg_color, color, wave_amplitude);

    FragColor = vec4(final_color, 1.0);
}
