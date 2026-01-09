#ifndef __CONFIG_H__
#define __CONFIG_H__

/* GLFW */
#define GLFW_WINDOW_WIDTH 800
#define GLFW_WINDOW_HEIGHT 600

/* Filepaths */
// TODO: externalize the shader path at build time
// There's probably a library for this shit or something
#define CONFIG_SHADER_PATH "src/shaders/"
#define CONFIG_SHADER_VERTEX "vertex.glsl"
#define CONFIG_SHADER_FRAGMENT "fragment.glsl"

#endif
