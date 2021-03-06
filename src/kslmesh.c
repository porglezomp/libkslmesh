#include "kslmesh.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

char *ksl_error_string;

ksl_mesh *ksl_make_mesh(ksl_vert *verts, ksl_line *lines,
			int vert_count, int line_count, int meter_size) {
  ksl_mesh *mesh = malloc(sizeof(ksl_mesh));
  *mesh = (ksl_mesh) {1, vert_count, line_count, meter_size, verts, lines};
  return mesh;
}
void ksl_retain_mesh(ksl_mesh *mesh) {
  mesh->refcount++;
}
void ksl_release_mesh(ksl_mesh *mesh) {
  mesh->refcount--;
  if (mesh->refcount <= 0) free(mesh);
}

ksl_mesh_handle ksl_make_handle(ksl_mesh *mesh) {
  ksl_retain_mesh(mesh);
  return (ksl_mesh_handle) {mesh};
}

void ksl_release_handle(ksl_mesh_handle handle) {
  // Decrement the reference of the internal mesh
  ksl_release_mesh(handle.shared_mesh);
}

void ksl_free_mesh_list(ksl_mesh_list *mesh_list) {
  for (int i = 0; i < mesh_list->count; i++) {
    ksl_release_mesh(mesh_list->meshes[i]);
  }
  free(mesh_list);
}

static ksl_mesh *parse_mesh(FILE *fd) {
  // Read out the extra whitespace
  fscanf(fd, " ");
  char mesh_word[4];
  fread(mesh_word, sizeof(char), 4, fd);
  if (strncmp("mesh", mesh_word, 4) != 0) {
    ksl_error_string = "KSL parse error: expected `mesh` when parsing mesh";
    return NULL;
  }

  int meter_size;
  if (fscanf(fd, " meter %d", &meter_size) == 0) {
    ksl_error_string = "KSL parse error: missing units `meter <number>`";
    return NULL;
  }

  int vert_count;
  if (fscanf(fd, " verts %d xy", &vert_count) == 0) {
    ksl_error_string = "KSL parse error: missing vert count `verts <number>`";
    return NULL;
  }

  ksl_vert *verts = malloc(vert_count*sizeof(ksl_vert));
  for (int i = 0; i < vert_count; i++) {
    short x, y;
    if (fscanf(fd, " %hd %hd", &x, &y) < 2) {
      ksl_error_string = "KSL parse error: expected an integer in verts";
      free(verts);
      return NULL;
    }
    verts[i] = (ksl_vert) {x, y};
  }

  int line_count;
  if (fscanf(fd, " lines %d", &line_count) == 0) {
    ksl_error_string = "KSL parse error: missing line count `lines <number>`";
    free(verts);
    return NULL;
  }

  ksl_line *lines = malloc(line_count*sizeof(ksl_line));
  for (int i = 0; i < line_count; i++) {
    unsigned short x, y;
    if (fscanf(fd, " %hu %hu", &x, &y) < 2) {
      ksl_error_string = "KSL parse error: expected an integer in lines";
      free(verts);
      free(lines);
      return NULL;
    }
    lines[i] = (ksl_line) {x, y};
  }

  return ksl_make_mesh(verts, lines, vert_count, line_count, meter_size);
}

ksl_mesh_list *ksl_load_meshes(const char *fname, ksl_mesh_list *appendto) {
  FILE *fd = fopen(fname, "r");
  if (fd == 0) {
    ksl_error_string = "KSL error: couldn't find file";
    return NULL;
  }

  char magic_word[5];
  fread(magic_word, sizeof(char), 5, fd);
  if (strncmp("ksl01", magic_word, 5) != 0) {
    ksl_error_string = "KSL parse error: missing magic number `ksl01`";
    return NULL;
  }

  int major, minor;
  if (fscanf(fd, " version %d.%d", &major, &minor) < 2) {
    ksl_error_string = "KSL parse error: missing `version <num>.<num>`";
    return NULL;
  }

  int num_meshes;  
  if (fscanf(fd, " meshes %d", &num_meshes) == 0) {
    ksl_error_string = "KSL parse error: missing `meshes <number>`";
    return NULL;
  }

  int first_index = 0;
  if (appendto == NULL) {
    // Allocate a mesh_list
    appendto = (ksl_mesh_list*) malloc(sizeof(*appendto));
    if (appendto == NULL) {
      ksl_error_string = "KSL error: malloc failed";
      return NULL;
    }

    // Allocate space for the meshes
    appendto->capacity = appendto->count = num_meshes;
    appendto->meshes = malloc(sizeof(ksl_mesh*) * appendto->capacity);
    if (appendto->meshes == NULL) {
      ksl_error_string = "KSL error: malloc failed";
      free(appendto);
      return NULL;
    }
  } else {
    // There's already an appendto, we just need to add more space
    first_index = appendto->count;
    appendto->count = appendto->capacity = first_index + num_meshes;
    appendto->meshes = realloc(appendto->meshes,
			       sizeof(ksl_mesh*) * appendto->capacity);
    if (appendto->meshes == NULL) {
      ksl_error_string = "KSL error: realloc failed";
      ksl_free_mesh_list(appendto);
      return NULL;
    }
  }
  int final_count = appendto->count;

  for (int i = first_index; i < final_count; i++) {
    ksl_mesh *mesh = parse_mesh(fd);
    if (mesh == NULL) return NULL;
    appendto->meshes[i] = mesh;
  }

  return appendto;
}

char *ksl_get_error() {
  return ksl_error_string;
}
