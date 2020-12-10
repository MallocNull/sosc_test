#ifndef OKUU_MESH_H
#define OKUU_MESH_H

#include <stdio.h>
#include <stdlib.h>

typedef struct {

} mesh_t;

mesh_t* mesh_load(const char*);
void mesh_unload(mesh_t*);

#endif
