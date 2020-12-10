#include "mesh.h"

mesh_t* mesh_load(const char* file) {
    FILE* fp = fopen(file, "rb");
    if(fp == NULL)
        return NULL;

    enum { VERTEX_CNT = 0, NORMAL_CNT, TEX_CNT, FACE_CNT };
    int counts[4] = {0, 0, 0, 0};
    char line[1024];

    for(;;) {
        fgets(line, 1024, fp);
    }
}