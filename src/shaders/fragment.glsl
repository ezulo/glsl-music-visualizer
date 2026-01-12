#version 330 core

out vec4 FragColor;

uniform float u_time;
uniform vec2 u_resolution;

void main() {
    vec2 uv_global, uv_local;
    vec3 color, bg_color, final_color;
    float dist_global, dist_local, wave_amplitude;
    // Normalize coordinates to [-1, 1] with aspect ratio correction
    uv_global = (gl_FragCoord.xy * 2.0 - u_resolution) / min(u_resolution.x, u_resolution.y);
    dist_global = length(uv_global);

    uv_local = uv_global;
    uv_local *= 2.0f;     // < 1.1x,              1.1y >
    uv_local = fract(uv_local); // < fract(1.1x),       fract(1.1y) >
    uv_local -= 0.5;      // < fract(1.1x) - 0.5, fract(1.1y) - 0.5 >

    dist_local = length(uv_local);

    // Wave amplitude
    wave_amplitude = 0.5f + sin(dist_local * 8.0f + u_time * 2.0f) / 2.0f;
    wave_amplitude = step(0.9f, wave_amplitude);

    // Animate color with time
    color = vec3(
        0.5 * sin(dist_global + u_time),
        0.5 * sin(dist_global + u_time + 2.094),
        0.5 * sin(dist_global + u_time + 4.188)
    );

    // Background color
    bg_color = vec3(0.1, 0.1, 0.15);

    // Mix circle and background
    final_color = mix(bg_color, color, wave_amplitude);

    FragColor = vec4(final_color, 1.0);
}
