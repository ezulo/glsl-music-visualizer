#include "shader.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

struct ShaderProgram {
  GLuint program;
  char name[256];
};

static std::vector<ShaderProgram> shaders;
static int current_shader = 0;
static char shader_dir_path[512];

static char *read_file(const char *path) {
  FILE *file = fopen(path, "rb");
  if (!file) {
    fprintf(stderr, "Failed to open file: %s\n", path);
    return NULL;
  }

  fseek(file, 0, SEEK_END);
  long length = ftell(file);
  fseek(file, 0, SEEK_SET);

  char *buffer = (char *)malloc(length + 1);
  if (!buffer) {
    fclose(file);
    return NULL;
  }

  fread(buffer, 1, length, file);
  buffer[length] = '\0';
  fclose(file);

  return buffer;
}

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

static GLuint create_shader_program(const char *vert_source,
                                    const char *frag_source) {
  GLuint vert_shader = compile_shader(GL_VERTEX_SHADER, vert_source);
  GLuint frag_shader = compile_shader(GL_FRAGMENT_SHADER, frag_source);

  if (!vert_shader || !frag_shader) {
    if (vert_shader) glDeleteShader(vert_shader);
    if (frag_shader) glDeleteShader(frag_shader);
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
    glDeleteShader(vert_shader);
    glDeleteShader(frag_shader);
    return 0;
  }

  glDeleteShader(vert_shader);
  glDeleteShader(frag_shader);

  return program;
}

static bool ends_with(const char *str, const char *suffix) {
  size_t str_len = strlen(str);
  size_t suffix_len = strlen(suffix);
  if (suffix_len > str_len) return false;
  return strcmp(str + str_len - suffix_len, suffix) == 0;
}

void shader_init(const char *vertex_dir, const char *vertex_file,
                 const char *fragment_dir) {
  strncpy(shader_dir_path, fragment_dir, sizeof(shader_dir_path) - 1);

  // Read vertex shader once
  char vert_path[512];
  snprintf(vert_path, sizeof(vert_path), "%s%s", vertex_dir, vertex_file);
  char *vert_source = read_file(vert_path);
  if (!vert_source) {
    fprintf(stderr, "Failed to load vertex shader: %s\n", vert_path);
    return;
  }

  // Scan fragment directory for fragment shaders
  DIR *dir = opendir(fragment_dir);
  if (!dir) {
    fprintf(stderr, "Failed to open fragment shader directory: %s\n",
            fragment_dir);
    free(vert_source);
    return;
  }

  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    // Skip non-.glsl files
    if (!ends_with(entry->d_name, ".glsl")) continue;

    char frag_path[512];
    snprintf(frag_path, sizeof(frag_path), "%s%s", fragment_dir, entry->d_name);

    char *frag_source = read_file(frag_path);
    if (!frag_source) continue;

    GLuint program = create_shader_program(vert_source, frag_source);
    free(frag_source);

    if (program) {
      ShaderProgram sp;
      sp.program = program;
      // Store name without .glsl extension
      strncpy(sp.name, entry->d_name, sizeof(sp.name) - 1);
      char *ext = strstr(sp.name, ".glsl");
      if (ext) *ext = '\0';

      shaders.push_back(sp);
      printf("Loaded shader: %s\n", sp.name);
    }
  }

  closedir(dir);
  free(vert_source);

  if (shaders.empty()) {
    fprintf(stderr, "No fragment shaders found in %s\n", fragment_dir);
  }
}

void shader_cleanup(void) {
  for (auto &sp : shaders) {
    glDeleteProgram(sp.program);
  }
  shaders.clear();
}

GLuint shader_get_current(void) {
  if (shaders.empty()) return 0;
  return shaders[current_shader].program;
}

const char *shader_get_current_name(void) {
  if (shaders.empty()) return "none";
  return shaders[current_shader].name;
}

int shader_get_count(void) {
  return (int)shaders.size();
}

void shader_next(void) {
  if (shaders.empty()) return;
  current_shader = (current_shader + 1) % shaders.size();
}

void shader_prev(void) {
  if (shaders.empty()) return;
  current_shader = (current_shader - 1 + shaders.size()) % shaders.size();
}
