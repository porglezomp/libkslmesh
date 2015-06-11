#ifndef _KSL_MESH_H
#define _KSL_MESH_H

#ifndef GLushort
#define GLushort unsigned short
#endif
#ifndef GLshort
#define GLshort short
#endif

typedef struct { GLshort x, y; } ksl_vert;
typedef struct { GLushort start, end; } ksl_line;

// Represents a reference counted mesh which may be shared
typedef struct {
  int refcount;
  int vert_count;
  int line_count;
  int meter_size;
  ksl_vert *verts;
  ksl_line *lines;
} ksl_mesh;

// Use this to point to a mesh
typedef struct {
  ksl_mesh *mesh;
} ksl_shared_mesh;

// For holding multiple meshes
typedef struct {
  int count;
  int capacity;
  ksl_mesh **meshes;
} ksl_mesh_list;

// Manage reference counts on a mesh
void ksl_retain(ksl_mesh*);
void ksl_release(ksl_mesh*);

// Creates a ksl_shared_mesh from a ksl_mesh*, incrementing the mesh's refcount
ksl_shared_mesh ksl_make_shared(ksl_mesh*);

// Delete a ksl_shared_mesh, decrementing the refcount on the data source
void ksl_release_shared(ksl_shared_mesh);

// Free a mesh list and release all of its contained meshes
void ksl_free_mesh_list(ksl_mesh_list*);

// NOTE: Takes ownership of the vertex and line arrays, if you don't want this
// then perform a memcpy beforehand
ksl_mesh *ksl_make_mesh(ksl_vert*, ksl_line*,
			int vert_count, int line_count, int meter_size);

// Parse meshes from a file and return a mesh list
// If you pass a mesh list then the new meshes will be appended to the end,
// if you pass NULL then a new mesh list will be created
// You have responsibility for freeing the mesh list
ksl_mesh_list *ksl_load_meshes(const char *fname, ksl_mesh_list *appendto);

// Return the error string generated when a method fails
char *ksl_get_error();

#endif
