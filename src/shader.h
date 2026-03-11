#ifndef __SHADER_H__
#define __SHADER_H__

#include <glad/gl.h>

void shader_init(const char *vertex_dir, const char *vertex_file,
                 const char *fragment_dir);
void shader_cleanup(void);

GLuint shader_get_current(void);
const char *shader_get_current_name(void);
int shader_get_count(void);

void shader_next(void);
void shader_prev(void);

#endif
