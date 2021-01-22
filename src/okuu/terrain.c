#include "terrain.h"

#define _TOP(A) (__MAX(0, A - CHUNK_SIZE / 2))

#define _HGT_AT(A, X, Y) (A->bmps[0]->pixels[Y][X][_R_] / 10.f)
#define _RWCOL_AT(A, X, Y) (A->bmps[1]->pixels[Y][X])
#define _RCOL_AT(A, X, Y, C) (_RWCOL_AT(A, X, Y)[C])
#define _COL_AT(A, X, Y, C) (_RCOL_AT(A, X, Y, C) / 255.f)
#define _COL_EQ(A, X1, Y1, X2, Y2) \
    (color_eqp(A->bmps[1]->pixels[Y1][X1], A->bmps[1]->pixels[Y2][X2]))

float _avg_hgt(terrain_t* map, int x, int y) {
    return
        (_HGT_AT(map, x, y) + _HGT_AT(map, x + 1, y) +
         _HGT_AT(map, x, y + 1) + _HGT_AT(map, x + 1, y + 1))
        / 4.f;
}

terrain_t* terrain_load
    (const char* heights, const char* colors,
     const color_t* paths, int path_cnt,
     int center_x, int center_y)
{
    int top_x = _TOP(center_x),
        top_y = _TOP(center_y);

    bmp_t* heights_bmp =
        bmp_load_chunk(heights, top_x, top_y, CHUNK_SIZE, CHUNK_SIZE);
    if(heights_bmp == NULL)
        return NULL;

    bmp_t* colors_bmp =
        bmp_load_chunk(colors, top_x, top_y, CHUNK_SIZE, CHUNK_SIZE);
    if(colors_bmp == NULL) {
        bmp_unload(heights_bmp);
        return NULL;
    }

    terrain_t* terrain = malloc(sizeof(terrain_t));
    terrain->bmps[0] = heights_bmp;
    terrain->bmps[1] = colors_bmp;
    terrain->top_x = terrain->top_y = -1;

    terrain->pick_dist = 20.f;
    terrain->pick_depth = 10;

    terrain->paths = NULL;
    terrain->path_cnt = 0;

    if(paths != NULL && path_cnt > 0) {
        terrain->paths = malloc(sizeof(color_t) * path_cnt);
        memcpy(terrain->paths, paths, sizeof(color_t) * path_cnt);
        terrain->path_cnt = path_cnt;
    }

    terrain_move(terrain, center_x, center_y);
    return terrain;
}

void terrain_move(terrain_t* terrain, int center_x, int center_y) {
    int new_terrain =
        terrain->top_x == -1
     || terrain->top_y == -1;

    terrain->top_x = _TOP(center_x);
    terrain->top_y = _TOP(center_y);

    if(new_terrain) {
        glGenVertexArrays(1, &terrain->vao);
        glGenBuffers(TERR_BUFFS, terrain->buffers);
    } else {
        bmp_reload_chunk
            (terrain->bmps[0], terrain->top_x, terrain->top_y,
             CHUNK_SIZE, CHUNK_SIZE);
        bmp_reload_chunk
            (terrain->bmps[1], terrain->top_x, terrain->top_y,
             CHUNK_SIZE, CHUNK_SIZE);
    }

    terrain->width = terrain->bmps[0]->width;
    terrain->height = terrain->bmps[0]->height;

    const int data_length = 12 * 3 * (CHUNK_SIZE - 1) * (CHUNK_SIZE - 1);
    terrain->tri_cnt = 4 * (terrain->width - 1) * (terrain->height - 1);
    float data[data_length];
    vec3 dir1, dir2, norm;

    for(int i = 0; i < TERR_BUFFS; ++i) {
        int cnt = i == 1 ? 2 : 3;
        for(int y = 0; y < terrain->height - 1; ++y) {
            for(int x = 0; x < terrain->width - 1; ++x) {
                int at = (y * (terrain->width - 1) * 12 * cnt) + (x * 12 * cnt);

                switch(i) {
                case 0:
                    data[at]      = x;
                    data[at + 1]  = terrain->heights[y][x] =
                        _HGT_AT(terrain, x, y);
                    data[at + 2]  = y;

                    data[at + 3]  = x + 0.5f;
                    data[at + 4]  = _avg_hgt(terrain, x, y);
                    data[at + 5]  = y + 0.5f;

                    data[at + 6]  = x + 1;
                    data[at + 7]  = _HGT_AT(terrain, x + 1, y);
                    data[at + 8]  = y;


                    data[at + 9]  = x + 1;
                    data[at + 10] = _HGT_AT(terrain, x + 1, y);
                    data[at + 11] = y;

                    data[at + 12] = x + 0.5f;
                    data[at + 13] = _avg_hgt(terrain, x, y);
                    data[at + 14] = y + 0.5f;

                    data[at + 15] = x + 1;
                    data[at + 16] = _HGT_AT(terrain, x + 1, y + 1);
                    data[at + 17] = y + 1;


                    data[at + 18] = x;
                    data[at + 19] = _HGT_AT(terrain, x, y + 1);
                    data[at + 20] = y + 1;

                    data[at + 21] = x + 1;
                    data[at + 22] = _HGT_AT(terrain, x + 1, y + 1);
                    data[at + 23] = y + 1;

                    data[at + 24] = x + 0.5f;
                    data[at + 25] = _avg_hgt(terrain, x, y);
                    data[at + 26] = y + 0.5f;


                    data[at + 27] = x;
                    data[at + 28] = _HGT_AT(terrain, x, y);
                    data[at + 29] = y;

                    data[at + 30]  = x;
                    data[at + 31] = _HGT_AT(terrain, x, y + 1);
                    data[at + 32] = y + 1;

                    data[at + 33] = x + 0.5f;
                    data[at + 34] = _avg_hgt(terrain, x, y);
                    data[at + 35] = y + 0.5f;
                    break;
                case 1:
                    data[at]      = 0.f;
                    data[at + 1]  = 0.f;

                    data[at + 2]  = .5f;
                    data[at + 3]  = .5f;

                    data[at + 4]  = 1.f;
                    data[at + 5]  = 0.f;


                    data[at + 6]  = 1.f;
                    data[at + 7]  = 0.f;

                    data[at + 8]  = .5f;
                    data[at + 9]  = .5f;

                    data[at + 10] = 1.f;
                    data[at + 11] = 1.f;


                    data[at + 12] = 0.f;
                    data[at + 13] = 1.f;

                    data[at + 14] = 1.f;
                    data[at + 15] = 1.f;

                    data[at + 16] = .5f;
                    data[at + 17] = .5f;


                    data[at + 18] = 0.f;
                    data[at + 19] = 0.f;

                    data[at + 20] = 0.f;
                    data[at + 21] = 1.f;

                    data[at + 22] = .5f;
                    data[at + 23] = .5f;
                    break;
                case 2:
                    glm_vec3_sub(
                        (vec3){ x, _HGT_AT(terrain, x, y), y },
                        (vec3){ x + 0.5f, _avg_hgt(terrain, x, y), y + 0.5f },
                        dir1
                    );
                    glm_vec3_sub(
                        (vec3){ x, _HGT_AT(terrain, x, y), y },
                        (vec3){ x + 1, _HGT_AT(terrain, x + 1, y), y },
                        dir2
                    );
                    glm_vec3_cross(dir1, dir2, norm);
                    glm_vec3_normalize(norm);
                    memcpy(&data[at], norm, 3 * sizeof(float));
                    memcpy(&data[at + 3], norm, 3 * sizeof(float));
                    memcpy(&data[at + 6], norm, 3 * sizeof(float));

                    glm_vec3_sub(
                        (vec3){ x + 1, _HGT_AT(terrain, x + 1, y), y },
                        (vec3){ x + 0.5f, _avg_hgt(terrain, x, y), y + 0.5f },
                        dir1
                    );
                    glm_vec3_sub(
                        (vec3){ x + 1, _HGT_AT(terrain, x + 1, y), y },
                        (vec3){ x + 1, _HGT_AT(terrain, x + 1, y + 1), y + 1 },
                        dir2
                    );
                    glm_vec3_cross(dir1, dir2, norm);
                    glm_vec3_normalize(norm);
                    memcpy(&data[at + 9], norm, 3 * sizeof(float));
                    memcpy(&data[at + 12], norm, 3 * sizeof(float));
                    memcpy(&data[at + 15], norm, 3 * sizeof(float));

                    glm_vec3_sub(
                        (vec3){ x + 1, _HGT_AT(terrain, x + 1, y + 1), y + 1 },
                        (vec3){ x + 0.5f, _avg_hgt(terrain, x, y), y + 0.5f },
                        dir1
                    );
                    glm_vec3_sub(
                        (vec3){ x + 1, _HGT_AT(terrain, x + 1, y + 1), y + 1 },
                        (vec3){ x, _HGT_AT(terrain, x, y + 1), y + 1 },
                        dir2
                    );
                    glm_vec3_cross(dir1, dir2, norm);
                    glm_vec3_normalize(norm);
                    memcpy(&data[at + 18], norm, 3 * sizeof(float));
                    memcpy(&data[at + 21], norm, 3 * sizeof(float));
                    memcpy(&data[at + 24], norm, 3 * sizeof(float));

                    glm_vec3_sub(
                        (vec3){ x, _HGT_AT(terrain, x, y), y },
                        (vec3){ x + 0.5f, _avg_hgt(terrain, x, y), y + 0.5f },
                        dir1
                    );
                    glm_vec3_sub(
                        (vec3){ x, _HGT_AT(terrain, x, y), y },
                        (vec3){ x, _HGT_AT(terrain, x, y + 1), y + 1 },
                        dir2
                    );
                    glm_vec3_cross(dir2, dir1, norm);
                    glm_vec3_normalize(norm);
                    memcpy(&data[at + 27], norm, 3 * sizeof(float));
                    memcpy(&data[at + 30], norm, 3 * sizeof(float));
                    memcpy(&data[at + 33], norm, 3 * sizeof(float));

                    glm_vec3_add(
                        data + at + 3,
                        data + at + 12,
                        data + at + 3
                    );
                    glm_vec3_add(
                        data + at + 3,
                        data + at + 33,
                        data + at + 3
                    );
                    glm_vec3_add(
                        data + at + 3,
                        data + at + 24,
                        data + at + 3
                    );
                    glm_vec3_normalize(data + at + 3);
                    memcpy(data + at + 12, data + at + 3, 3 * sizeof(float));
                    memcpy(data + at + 24, data + at + 3, 3 * sizeof(float));
                    memcpy(data + at + 33, data + at + 3, 3 * sizeof(float));
                    break;
                case 3:
                    for(int j = 0; j < 12; ++j) {
                        data[at + j * 3]     = _COL_AT(terrain, x, y, _R_);
                        data[at + j * 3 + 1] = _COL_AT(terrain, x, y, _G_);
                        data[at + j * 3 + 2] = _COL_AT(terrain, x, y, _B_);
                    }

                    if(terrain->paths == NULL)
                        continue;

                    int found = 0;
                    for(int j = 0; j < terrain->path_cnt; ++j) {
                        if(color_eqp(terrain->paths[j],
                            _RWCOL_AT(terrain, x, y)))
                        {
                            found = 1;
                            break;
                        }
                    }

                    if(!found)
                        continue;

                    if(x == 0 || y == 0)
                        continue;

                    int neighbors =
                        _COL_EQ(terrain, x, y, x, y - 1) +
                        _COL_EQ(terrain, x, y, x + 1, y) +
                        _COL_EQ(terrain, x, y, x, y + 1) +
                        _COL_EQ(terrain, x, y, x - 1, y);

                    if(neighbors > 2)
                        continue;

                    const int coords[][2] = {
                        {0, -1}, {1, 0},
                        {0, 1}, {-1, 0}
                    };

                    for(int j = 0; j < 4; ++j) {
                        if(!_COL_EQ(terrain,
                                x, y,
                                x + coords[j][0],
                                y + coords[j][1]))
                        {
                            data[at + j * 9] = _COL_AT(terrain,
                                x + coords[j][0],
                                y + coords[j][1], _R_
                            );

                            data[at + j * 9 + 1] = _COL_AT(terrain,
                                x + coords[j][0],
                                y + coords[j][1], _G_
                            );

                            data[at + j * 9 + 2] = _COL_AT(terrain,
                                x + coords[j][0],
                                y + coords[j][1], _B_
                            );

                            memcpy(data + at + j * 9 + 3, data + at + j * 9, 3 * sizeof(float));
                            memcpy(data + at + j * 9 + 6, data + at + j * 9, 3 * sizeof(float));
                        }
                    }

                    break;
                }
            }
        }

        glBindBuffer(GL_ARRAY_BUFFER, terrain->buffers[i]);
        if(new_terrain) {
            glBufferData(
                GL_ARRAY_BUFFER,
                data_length * sizeof(float),
                data,
                GL_DYNAMIC_DRAW
            );

            glBindVertexArray(terrain->vao); {
                glEnableVertexAttribArray(i);
                glVertexAttribPointer(i, cnt, GL_FLOAT,
                    i == 2 ? GL_TRUE : GL_FALSE, 0, (void*)0);
            } glBindVertexArray(0);
        } else {
            glBufferSubData(
                GL_ARRAY_BUFFER, 0,
                terrain->tri_cnt * 3 * 3 * sizeof(float),
                data
            );
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

float terrain_height(terrain_t* terrain, int x, int y) {
    x -= terrain->top_x;
    y -= terrain->top_y;

    if(x < 0 || y < 0 || x >= terrain->width || y >= terrain->height)
        return NAN;

    return terrain->heights[y][x];
}

void terrain_pick_params(terrain_t* terrain, float distance, int depth) {
    terrain->pick_dist = distance;
    terrain->pick_depth = depth;
}

int terrain_pick
    (terrain_t* terrain, vec3 ray, vec3 camera, vec3 out)
{
    int intersect = 0;
    float t = terrain->pick_dist / 2.f, delta = t;

    for(int i = 0; i < terrain->pick_depth; ++i) {
        delta /= 2;
        glm_vec3_scale(ray, t, out);
        glm_vec3_add(out, camera, out);

        float height = terrain_height(terrain, out[0], out[2]);
        if(height == NAN)
            t = t - delta;
        else {
            if(height < out[1])
                t = t + delta;
            else {
                intersect = 1;
                t = t - delta;
            }

            out[1] = height;
        }
    }

    return intersect;
}

void terrain_unload(terrain_t* terrain) {
    bmp_unload(terrain->bmps[0]);
    bmp_unload(terrain->bmps[1]);
    free(terrain);
}