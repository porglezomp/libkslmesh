#include "kslmesh_gl.h"

#include <stdlib.h>
#include <unistd.h>

#include "kslmesh.h"
#include "GLES/gl.h"

ksl_gl_mesh *ksl_make_gl_mesh(ksl_mesh *mesh) {
  if (mesh == NULL) return NULL;
  ksl_gl_mesh *glmesh = malloc(sizeof(*glmesh));
  if (glmesh == NULL) return NULL;

  glmesh->refcount = 0;
  ksl_retain_gl_mesh(glmesh);

  glGenBuffers(1, &glmesh->vbo);
  glBindBuffer(GL_ARRAY_BUFFER, glmesh->vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(short)*mesh->vert_count*2,
	       mesh->verts, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glGenBuffers(1, &glmesh->ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glmesh->ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(short)*mesh->line_count*2,
	       mesh->lines, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  glmesh->element_count = mesh->line_count*2;
  glmesh->meter_size = mesh->meter_size;
  return glmesh;
}

void ksl_retain_gl_mesh(ksl_gl_mesh *mesh) {
  mesh->refcount += 1;
}

void ksl_release_gl_mesh(ksl_gl_mesh *mesh) {
  mesh->refcount -= 1;
  if (mesh->refcount == 0) {
    glDeleteBuffers(1, &mesh->vbo);
    glDeleteBuffers(1, &mesh->ibo);
    free(mesh);
  }
}

ksl_gl_mesh_handle ksl_make_gl_handle(ksl_gl_mesh *mesh) {
  ksl_retain_gl_mesh(mesh);
  return (ksl_gl_mesh_handle) {mesh};
}

// Declare a handle as released, and release the reference counted contents
void ksl_release_gl_handle(ksl_gl_mesh_handle handle) {
  ksl_release_gl_mesh(handle.shared_mesh);
}

// Draw the OpenGL mesh
void ksl_draw_gl_mesh(ksl_gl_mesh *mesh) {
  // Boring setup, push matrix so we can restore original scale
  glPushMatrix();
  glEnableClientState(GL_VERTEX_ARRAY);

  // Scale from integer units into meters
  float scale = 1.0 / mesh->meter_size;
  glScalef(scale, scale, scale);

  // Bind the vertex and index buffers for drawing
  glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo);

  // Draw the mesh from the VBOs
  glVertexPointer(2, GL_SHORT, 0, 0);
  glDrawElements(GL_LINES, mesh->element_count, GL_UNSIGNED_SHORT, 0);
  
  // Boring cleanup, make errors appear sooner
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glDisableClientState(GL_VERTEX_ARRAY);
  glPopMatrix();
}
