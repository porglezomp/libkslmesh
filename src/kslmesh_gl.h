#ifndef _KSL_GL_MESH_H
#define _KSL_GL_MESH_H

#include "kslmesh.h"

// Represents a mesh optimized for drawing by OpenGL. It only contains handles,
// the actual mesh data is stored on the graphics card.
typedef struct {
  int refcount;
  int vbo, ibo;
  int element_count;
  int meter_size;
} ksl_gl_mesh;

// Wraps a pointer into the reference counted OpenGL mesh
typedef struct {
  ksl_gl_mesh *shared_mesh;
} ksl_gl_mesh_handle;

// Creates an OpenGL mesh by creating VBOs
ksl_gl_mesh *ksl_make_gl_mesh(ksl_mesh*);
// Reference count manipulations
void ksl_retain_gl_mesh(ksl_gl_mesh*);
void ksl_release_gl_mesh(ksl_gl_mesh*);

ksl_gl_mesh_handle ksl_make_gl_handle(ksl_gl_mesh*);

// Declare a handle as released, and release the reference counted contents
void ksl_gl_release_handle(ksl_gl_mesh_handle);

// Draw the OpenGL mesh
void ksl_draw_gl_mesh(ksl_gl_mesh*);

#endif
