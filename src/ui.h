#ifndef __UI_H__
#define __UI_H__

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

void ui_init(GLFWwindow *window);
void ui_render(void);
void ui_cleanup(void);

#endif
