#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

#include "config.h"

#define SHADER_VERTEX CONFIG_SHADER_PATH CONFIG_SHADER_VERTEX
#define SHADER_FRAGMENT CONFIG_SHADER_PATH CONFIG_SHADER_FRAGMENT

static char *read_file(const char *path) {
  FILE *file = fopen(path, "rb");
  if (!file) {
    fprintf(stderr, "Failed to open file: %s\n", path);
    return NULL;
  }

  fseek(file, 0, SEEK_END);
  long length = ftell(file);
  fseek(file, 0, SEEK_SET);

  char *buffer = malloc(length + 1);
  if (!buffer) {
    fclose(file);
    return NULL;
  }

  fread(buffer, 1, length, file);
  buffer[length] = '\0';
  fclose(file);

  return buffer;
}

/*
 * Compiles a shader for the GPU
 */
static GLuint compile_shader(GLenum type, const char *source) {
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &source, NULL);
  glCompileShader(shader);

  GLint success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    char log[512];
    glGetShaderInfoLog(shader, sizeof(log), NULL, log);
    fprintf(stderr, "Shader compilation error: %s\n", log);
    return 0;
  }

  return shader;
}

/*
 * Links our built shaders, creates our shader program, and frees up shader
 * objects
 */
static GLuint create_shader_program(const char *vert_path,
                                    const char *frag_path) {
  char *vert_source = read_file(vert_path);
  char *frag_source = read_file(frag_path);

  if (!vert_source || !frag_source) {
    free(vert_source);
    free(frag_source);
    return 0;
  }

  GLuint vert_shader = compile_shader(GL_VERTEX_SHADER, vert_source);
  GLuint frag_shader = compile_shader(GL_FRAGMENT_SHADER, frag_source);

  free(vert_source);
  free(frag_source);

  if (!vert_shader || !frag_shader) {
    return 0;
  }

  GLuint program = glCreateProgram();
  glAttachShader(program, vert_shader);
  glAttachShader(program, frag_shader);
  glLinkProgram(program);

  GLint success;
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success) {
    char log[512];
    glGetProgramInfoLog(program, sizeof(log), NULL, log);
    fprintf(stderr, "Program linking error: %s\n", log);
    return 0;
  }

  glDeleteShader(vert_shader);
  glDeleteShader(frag_shader);

  return program;
}

int main(void) {
  if (!glfwInit()) {
    fprintf(stderr, "Failed to initialize GLFW\n");
    return -1;
  }

  /* Initialize GLFW Window */

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window = glfwCreateWindow(GLFW_WINDOW_WIDTH, GLFW_WINDOW_HEIGHT,
                                        "GLSL Visualizer", NULL, NULL);
  if (!window) {
    fprintf(stderr, "Failed to create GLFW window\n");
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  /* Load OpenGL function pointers for our context */

  if (glewInit() != GLEW_OK) {
    fprintf(stderr, "Failed to initialize GLEW\n");
    return -1;
  }

  printf("OpenGL Version: %s\n", glGetString(GL_VERSION));

  /* Create our geometry (in this case, two triangles) */

  float vertices[] = {-1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f};
  unsigned int indices[] = {0, 1, 2, 2, 3, 0};

  /* Initialize GPU buffers */

  GLuint VAO, VBO, EBO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
               GL_STATIC_DRAW);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  /* Create our OpenGL program */

  GLuint shader_program = create_shader_program(SHADER_VERTEX, SHADER_FRAGMENT);
  if (!shader_program) {
    fprintf(stderr, "Failed to create shader program\n");
    return -1;
  }

  GLint time_loc = glGetUniformLocation(shader_program, "u_time");
  GLint resolution_loc = glGetUniformLocation(shader_program, "u_resolution");

  while (!glfwWindowShouldClose(window)) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
      glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shader_program);
    glUniform1f(time_loc, (float)glfwGetTime());
    glUniform2f(resolution_loc, (float)width, (float)height);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteBuffers(1, &EBO);
  glDeleteProgram(shader_program);

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
