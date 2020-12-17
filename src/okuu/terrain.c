#include "terrain.h"

#define _HGT_AT(A, X, Y) (A->bmps[0]->pixels[Y][X].r / 10.f)
#define _COL_AT(A, X, Y, C) (A->bmps[1]->pixels[Y][X].C / 255.f)

terrain_t* terrain_load
    (const char* heights, const char* colors,
     int center_x, int center_y)
{
    int top_x = __MAX(0, center_x - CHUNK_SIZE / 2),
        top_y = __MAX(0, center_y - CHUNK_SIZE / 2);

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
    terrain->center_x = terrain->center_y = -1;

    terrain_move(terrain, center_x, center_y);
    return terrain;
}

void terrain_move(terrain_t* terrain, int center_x, int center_y) {
    int new_terrain = terrain->center_x == -1 || terrain->center_y == -1;

    if(new_terrain) {
        glGenVertexArrays(1, &terrain->vao);
        glGenBuffers(TERR_BUFFS, terrain->buffers);
    } else {
        int top_x = __MAX(0, center_x - CHUNK_SIZE / 2),
            top_y = __MAX(0, center_y - CHUNK_SIZE / 2);

        bmp_reload_chunk
            (terrain->bmps[0], top_x, top_y, CHUNK_SIZE, CHUNK_SIZE);
        bmp_reload_chunk
            (terrain->bmps[1], top_x, top_y, CHUNK_SIZE, CHUNK_SIZE);
    }

    terrain->center_x = center_x;
    terrain->center_y = center_y;

    const int data_length = 6 * 3 * (CHUNK_SIZE - 1) * (CHUNK_SIZE - 1);
    float data[data_length];
    vec3 dir1, dir2, norm;

    for(int i = 0; i < TERR_BUFFS; ++i) {
        for(int y = 0; y < CHUNK_SIZE - 1; ++y) {
            for(int x = 0; x < CHUNK_SIZE - 1; ++x) {
                int at = (y * CHUNK_SIZE * 6 * 3) + (x * 6 * 3);

                switch(i) {
                case 0:
                    data[at]      = x;
                    data[at + 1]  = _HGT_AT(terrain, x, y);
                    data[at + 2]  = y;

                    data[at + 3]  = x;
                    data[at + 4]  = _HGT_AT(terrain, x, y + 1);
                    data[at + 5]  = y + 1;

                    data[at + 6]  = x + 1;
                    data[at + 7]  = _HGT_AT(terrain, x + 1, y);
                    data[at + 8]  = y;

                    data[at + 9]  = x + 1;
                    data[at + 10] = _HGT_AT(terrain, x + 1, y + 1);
                    data[at + 11] = y + 1;

                    data[at + 12] = x + 1;
                    data[at + 13] = _HGT_AT(terrain, x + 1, y);
                    data[at + 14] = y;

                    data[at + 15] = x;
                    data[at + 16] = _HGT_AT(terrain, x, y + 1);
                    data[at + 17] = y + 1;
                    break;
                case 1:
                    glm_vec3_sub(
                        (vec3){ x + 1, _HGT_AT(terrain, x + 1, y), y },
                        (vec3){ x, _HGT_AT(terrain, x, y), y },
                        dir1
                    );
                    glm_vec3_sub(
                        (vec3){ x, _HGT_AT(terrain, x, y + 1), y + 1 },
                        (vec3){ x, _HGT_AT(terrain, x, y), y },
                        dir2
                    );
                    glm_vec3_cross(dir1, dir2, norm);
                    memcpy(&data[at], norm, 3 * sizeof(float));
                    memcpy(&data[at + 3], norm, 3 * sizeof(float));
                    memcpy(&data[at + 6], norm, 3 * sizeof(float));

                    glm_vec3_sub(
                        (vec3){ x + 1, _HGT_AT(terrain, x + 1, y + 1), y + 1 },
                        (vec3){ x + 1, _HGT_AT(terrain, x + 1, y), y },
                        dir1
                    );
                    glm_vec3_sub(
                        (vec3){ x + 1, _HGT_AT(terrain, x + 1, y + 1), y + 1 },
                        (vec3){ x, _HGT_AT(terrain, x, y + 1), y + 1 },
                        dir2
                    );
                    glm_vec3_cross(dir1, dir2, norm);
                    memcpy(&data[at + 9], norm, 3 * sizeof(float));
                    memcpy(&data[at + 12], norm, 3 * sizeof(float));
                    memcpy(&data[at + 15], norm, 3 * sizeof(float));
                    break;
                case 2:
                    data[at]      = _COL_AT(terrain, x, y, r);
                    data[at + 1]  = _COL_AT(terrain, x, y, g);
                    data[at + 2]  = _COL_AT(terrain, x, y, b);

                    data[at + 3]  = _COL_AT(terrain, x + 1, y, r);
                    data[at + 4]  = _COL_AT(terrain, x + 1, y, g);
                    data[at + 5]  = _COL_AT(terrain, x + 1, y, b);

                    data[at + 6]  = _COL_AT(terrain, x, y + 1, r);
                    data[at + 7]  = _COL_AT(terrain, x, y + 1, g);
                    data[at + 8]  = _COL_AT(terrain, x, y + 1, b);

                    data[at + 9]  = _COL_AT(terrain, x + 1, y + 1, r);
                    data[at + 10] = _COL_AT(terrain, x + 1, y + 1, g);
                    data[at + 11] = _COL_AT(terrain, x + 1, y + 1, b);

                    data[at + 12] = _COL_AT(terrain, x + 1, y, r);
                    data[at + 13] = _COL_AT(terrain, x + 1, y, g);
                    data[at + 14] = _COL_AT(terrain, x + 1, y, b);

                    data[at + 15] = _COL_AT(terrain, x, y + 1, r);
                    data[at + 16] = _COL_AT(terrain, x, y + 1, g);
                    data[at + 17] = _COL_AT(terrain, x, y + 1, b);
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
                glVertexAttribPointer(i, 3, GL_FLOAT,
                    i == 1 ? GL_TRUE : GL_FALSE, 0, (void*)0);
            } glBindVertexArray(0);
        } else {
            glBufferSubData(
                GL_ARRAY_BUFFER, 0,
                data_length * sizeof(float),
                data
            );
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void terrain_unload(terrain_t* terrain) {
    bmp_unload(terrain->bmps[0]);
    bmp_unload(terrain->bmps[1]);
    free(terrain);
}