#include "obb.h"

void obb_from_center(obb_t* obb, vec3 center, vec3 bounds) {
    vec3 cbound;

    glm_vec3_abs(bounds, cbound);
    glm_vec3_scale(cbound, 0.5f, cbound);
    glm_vec3_sub(center, cbound, obb->min);
    glm_vec3_add(center, cbound, obb->max);
}

void obb_from_min(obb_t* obb, vec3 min, vec3 bounds) {
    vec3 cbound;

    glm_vec3_abs(bounds, cbound);
    glm_vec3_copy(min, obb->min);
    glm_vec3_add(min, cbound, obb->max);
}

void obb_from_max(obb_t* obb, vec3 max, vec3 bounds) {
    vec3 cbound;

    glm_vec3_abs(bounds, cbound);
    glm_vec3_copy(max, obb->max);
    glm_vec3_sub(max, cbound, obb->min);
}

void obb_clear(obb_t* obb) {
    if(obb == NULL)
        return;

    obb->min[0] =
    obb->min[1] =
    obb->min[2] =
    obb->max[0] =
    obb->max[1] =
    obb->max[2] = NAN;
}

void obb_parse_point(obb_t* obb, vec3 pt) {
    if(obb == NULL)
        return;

    if(obb->min[0] == NAN) {
        glm_vec3_copy(pt, obb->min);
        glm_vec3_copy(pt, obb->max);
    } else {
        obb->min[0] = __MIN(obb->min[0], pt[0]);
        obb->min[1] = __MIN(obb->min[1], pt[1]);
        obb->min[2] = __MIN(obb->min[2], pt[2]);

        obb->max[0] = __MIN(obb->max[0], pt[0]);
        obb->max[1] = __MIN(obb->max[1], pt[1]);
        obb->max[2] = __MIN(obb->max[2], pt[2]);
    }
}