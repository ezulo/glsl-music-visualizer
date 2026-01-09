#version 330 core

out vec4 FragColor;

uniform float u_time;
uniform vec2 u_resolution;

void main() {
    // Normalize coordinates to [-1, 1] with aspect ratio correction
    vec2 uv = (gl_FragCoord.xy * 2.0 - u_resolution) / min(u_resolution.x, u_resolution.y);

    // Circle parameters
    float radius = 0.25;
    float dist = length(uv);

    // Smooth circle edge
    float circle = 1.0 - dist;

    // Animate color with time
    //vec3 color = vec3(
    //    0.5 + 0.5 * sin(u_time),
    //    0.5 + 0.5 * sin(u_time + 2.094),
    //    0.5 + 0.5 * sin(u_time + 4.188)
    //);

    // Color circle magenta
    vec3 color = vec3(circle, 0, 0.0);

    // Background color
    vec3 bg_color = vec3(0.1, 0.1, 0.15);

    // Mix circle and background
    //vec3 final_color = mix(bg_color, color, circle);
    vec3 final_color = color;

    FragColor = vec4(final_color, 1.0);
}
