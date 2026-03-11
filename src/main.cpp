#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/gl.h>
#include <stdio.h>

#include "audio.h"
#include "config.h"
#include "shader.h"
#include "ui.h"

/*
 * Resize FBO textures if window dimensions changed
 */
static void resize_fbo_textures(GLuint *fbo_texture, int width, int height,
                                int *last_width, int *last_height) {
  if (width != *last_width || height != *last_height) {
    for (int i = 0; i < 2; i++) {
      glBindTexture(GL_TEXTURE_2D, fbo_texture[i]);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
                   GL_UNSIGNED_BYTE, NULL);
    }
    *last_width = width;
    *last_height = height;
  }
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

  if (!gladLoadGL(glfwGetProcAddress)) {
    fprintf(stderr, "Failed to initialize GLAD\n");
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

  /* Load shaders */

  shader_init(CONFIG_SHADER_VERTEX_PATH, CONFIG_SHADER_VERTEX,
              CONFIG_SHADER_FRAGMENT_PATH);
  if (shader_get_count() == 0) {
    fprintf(stderr, "No shaders loaded\n");
    return -1;
  }

  /* Initialize audio capture */

  if (audio_init() != 0) {
    fprintf(stderr, "Warning: Audio initialization failed, continuing without "
                    "audio\n");
  }

  /* Create 1D texture for FFT data */
  GLuint fft_texture;
  glGenTextures(1, &fft_texture);
  glBindTexture(GL_TEXTURE_1D, fft_texture);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexImage1D(GL_TEXTURE_1D, 0, GL_R32F, audio_get_fft_size(), 0, GL_RED,
               GL_FLOAT, NULL);

  /* 2 Frame Buffer Objects for tracer effect */
  GLuint fbo[2], fbo_texture[2];
  int current_fbo = 0;

  glGenFramebuffers(2, fbo);
  glGenTextures(2, fbo_texture);

  for (int i = 0; i < 2; i++) {
    glBindTexture(GL_TEXTURE_2D, fbo_texture[i]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, GLFW_WINDOW_WIDTH,
                 GLFW_WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                    GL_CLAMP_TO_EDGE); // x dimension of texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                    GL_CLAMP_TO_EDGE); // y dimension of texture

    glBindFramebuffer(GL_FRAMEBUFFER, fbo[i]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           fbo_texture[i], 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      fprintf(stderr, "Framebuffer %d not complete\n", i);
      return -1;
    }
  }

  /* Unbind FBO, return to default framebuffer (screen) */
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  /* Initialize ImGui */
  ui_init(window);

  /* Tracer multiplier */
  float tracer_mult = 0.97f;

  /* Track framebuffer size for FBO resize */
  int last_width = GLFW_WINDOW_WIDTH, last_height = GLFW_WINDOW_HEIGHT;

  while (!glfwWindowShouldClose(window)) {
    /* Keybinds */
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
      glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
      tracer_mult += 0.001f;
      if (tracer_mult > 1.0f) {
        tracer_mult = 1.0f;
      }
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
      tracer_mult -= 0.001f;
      if (tracer_mult < 0.0f) {
        tracer_mult = 0.0f;
      }
    }

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    resize_fbo_textures(fbo_texture, width, height, &last_width, &last_height);
    glViewport(0, 0, width, height);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo[current_fbo]);
    glClear(GL_COLOR_BUFFER_BIT);

    /* Update audio and FFT */
    audio_update();

    /* Upload FFT data to texture */
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_1D, fft_texture);
    glTexSubImage1D(GL_TEXTURE_1D, 0, 0, audio_get_fft_size(), GL_RED, GL_FLOAT,
                    audio_get_fft_data());

    /* Upload framebuffer texture */
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, fbo_texture[1 - current_fbo]);

    /* Assign Uniforms */
    GLuint program = shader_get_current();
    glUseProgram(program);
    glUniform1f(glGetUniformLocation(program, "u_time"), (float)glfwGetTime());
    glUniform2f(glGetUniformLocation(program, "u_resolution"), (float)width, (float)height);
    glUniform1f(glGetUniformLocation(program, "u_tracer_mult"), (float)tracer_mult);
    glUniform1i(glGetUniformLocation(program, "u_fft"), 0);        // on GL_TEXTURE0 slot
    glUniform1i(glGetUniformLocation(program, "u_prev_frame"), 1); // on GL_TEXTURE1 slot

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    /* Unbind FBO, blit to screen */
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);

    current_fbo = 1 - current_fbo;

    /* Render UI overlay */
    ui_render();

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  audio_cleanup();
  ui_cleanup();
  shader_cleanup();

  glDeleteTextures(1, &fft_texture);
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteBuffers(1, &EBO);

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
