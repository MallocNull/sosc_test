#ifndef OKUU_OBB_H
#define OKUU_OBB_H

#include <cglm/cglm.h>
#include "koa/etc.h"

typedef struct {
    vec3 min, max;
} obb_t;

void obb_from_center(obb_t*, vec3, vec3);
void obb_from_min(obb_t*, vec3, vec3);
void obb_from_max(obb_t*, vec3, vec3);

void obb_clear(obb_t*);
void obb_parse_point(obb_t*, vec3);

//int obb_pick(obb_t*, ...)

#endif
