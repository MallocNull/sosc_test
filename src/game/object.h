#ifndef SOSC_OBJECT_H
#define SOSC_OBJECT_H

#include <cglm/cglm.h>
#include "okuu/mesh.h"

typedef struct {
    mesh_t* mesh;
    mat4 translate, scale, rotate;
} object_t;

void object_create(mesh_t*);

void object_reset_matrices(object_t*);
void object_reset_translate(object_t*);
void object_reset_scale(object_t*);
void object_reset_rotate(object_t*);

void object_translate(object_t*, vec3);
void object_scale(object_t*, vec3, float);
void object_scale_uni(object_t*, vec3, float);
void object_rotate(object_t*, vec3, float);
void object_
void object_model_matrix(object_t*, mat4);

void object_destroy(object_t*);

#endif
