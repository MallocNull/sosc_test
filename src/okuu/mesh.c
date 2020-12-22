#include "mesh.h"

typedef struct {
    float data[3];
} _pt_t;

typedef struct {
    uint32_t data[3][3];
} _tri_t;

int _populate_pts
    (_pt_t* data, FILE* fp, uint32_t length, int width)
{
    uint8_t buffer[4];
    int chk;

    for(uint32_t i = 0; i < length; ++i) {
        for(int j = 0; j < width; ++j) {
            chk = fread(buffer, 1, 4, fp);
            if(chk != 4)
                return 0;

            data[i].data[j] = *(float*)ltoh(buffer, 4);
        }
    }

    return 1;
}

int _populate_faces
    (_tri_t* data, FILE* fp, uint32_t at, int length, int flags)
{
    uint8_t buffer[4] = { 0 };
    int chk;

    for(int i = 0; i < 3; ++i) {
        for(int j = 0; j < 3; ++j) {
            if((flags & (1 << j)) == 0) {
                data[at].data[i][j] = 0;
                continue;
            }

            chk = fread(buffer, 1, length, fp);
            if(chk != length)
                return 0;

            data[at].data[i][j] = *(uint32_t *) ltoh(buffer, 4);
        }
    }

    return 1;
}

mesh_t* mesh_load(const char* file) {
    FILE* fp = fopen(file, "rb");
    if(fp == NULL)
        return NULL;

    enum { VERT_CNT = 0, TEX_CNT, NORM_CNT, FACE_CNT };
    uint32_t counts[4] = { 0 };

    const uint8_t magic[] = {0xDE, 0xAF, 0xB0, 0x0B};
    uint8_t buffer[4] = { 0 };
    fread(buffer, 1, 4, fp);

    if(memcmp(buffer, magic, 4) != 0) {
        fclose(fp);
        return NULL;
    }

    for(int i = 0; i < 4; ++i) {
        fread(buffer, 1, 4, fp);
        counts[i] = *(uint32_t*)ltoh(buffer, 4);
    }

    if(feof(fp) || counts[VERT_CNT] == 0 || counts[FACE_CNT] == 0) {
        fclose(fp);
        return NULL;
    }

    _pt_t  *verts = malloc(sizeof(_pt_t) * counts[VERT_CNT]),
           *texs  = malloc(sizeof(_pt_t) * counts[TEX_CNT]),
           *norms = malloc(sizeof(_pt_t) * counts[NORM_CNT]);
    _tri_t *faces = malloc(sizeof(_tri_t) * counts[FACE_CNT]);

    int chk = 1;

    chk &= _populate_pts(verts, fp, counts[VERT_CNT], 3);
    if(counts[TEX_CNT] > 0)
        chk &= _populate_pts(texs, fp, counts[TEX_CNT], 2);
    if(counts[NORM_CNT] > 0)
        chk &= _populate_pts(norms, fp, counts[NORM_CNT], 3);

    int index_length = counts[FACE_CNT] <= 0xFFFF ? 2 : 4;
    for(uint32_t i = 0; i < counts[FACE_CNT]; ++i) {
        if(counts[TEX_CNT] > 0 && counts[NORM_CNT] > 0)
            chk &= _populate_faces(faces, fp, i, index_length, 0b111);
        else if(counts[TEX_CNT] > 0 && counts[NORM_CNT] == 0)
            chk &= _populate_faces(faces, fp, i, index_length, 0b110);
        else if(counts[TEX_CNT] == 0 && counts[NORM_CNT] > 0)
            chk &= _populate_faces(faces, fp, i, index_length, 0b101);
        else
            chk &= _populate_faces(faces, fp, i, index_length, 0b100);
    }

    fclose(fp);
    if(!chk) {
        mfree(0, verts, texs, norms, faces, NULL);
        return NULL;
    }

    float *verts_asm = malloc(sizeof(float) * counts[FACE_CNT] * 3 * 3),
          *texs_asm  = malloc(sizeof(float) * counts[FACE_CNT] * 3 * 2),
          *norms_asm = malloc(sizeof(float) * counts[FACE_CNT] * 3 * 3);
    mesh_t* mesh = malloc(sizeof(mesh_t));

    for(uint32_t i = 0; i < counts[FACE_CNT]; ++i) {
        for(int j = 0; j < 3; ++j) {
            if(faces[i].data[j][0] > counts[VERT_CNT] ||
               faces[i].data[j][1] > counts[TEX_CNT]  ||
               faces[i].data[j][2] > counts[NORM_CNT])
            {
                mfree(0,
                      verts_asm, texs_asm, norms_asm,
                      verts, texs, norms, faces, NULL
                );

                return NULL;
            }

            memcpy(
                &verts_asm[i * 3 *3 + 3 * j],
                verts[faces[i].data[j][0] - 1].data,
                3 * sizeof(float)
            );

            if(counts[TEX_CNT] > 0 && faces[i].data[j][1] != 0) {
                memcpy(
                    &texs_asm[i * 3 * 2 + 2 * j],
                    texs[faces[i].data[j][1] - 1].data,
                    2 * sizeof(float)
                );
            }

            if(counts[NORM_CNT] > 0 && faces[i].data[j][2] != 0) {
                memcpy(
                    &norms_asm[i * 3 * 3 + 3 * j],
                    norms[faces[i].data[j][2] - 1].data,
                    3 * sizeof(float)
                );
            }
        }
    }

    glGenBuffers(3, mesh->buffers);
    glGenVertexArrays(1, &mesh->vao);

    glBindBuffer(GL_ARRAY_BUFFER, mesh->buffers[0]);
    glBufferData(
        GL_ARRAY_BUFFER,
        counts[FACE_CNT] * 3 * 3 * sizeof(float),
        verts_asm,
        GL_STATIC_DRAW
    );

    glBindBuffer(GL_ARRAY_BUFFER, mesh->buffers[1]);
    glBufferData(
        GL_ARRAY_BUFFER,
        counts[FACE_CNT] * 3 * 2 * sizeof(float),
        texs_asm,
        GL_STATIC_DRAW
    );

    glBindBuffer(GL_ARRAY_BUFFER, mesh->buffers[2]);
    glBufferData(
        GL_ARRAY_BUFFER,
        counts[FACE_CNT] * 3 * 3 * sizeof(float),
        norms_asm,
        GL_STATIC_DRAW
    );

    mfree(0,
        verts_asm, texs_asm, norms_asm,
        verts, texs, norms, faces, NULL
    );

    glBindVertexArray(mesh->vao); {
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->buffers[0]);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->buffers[1]);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->buffers[2]);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    } glBindVertexArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    mesh->tri_cnt = counts[FACE_CNT];
    return mesh;
}

void mesh_bind(mesh_t* mesh) {
    glBindVertexArray(mesh->vao);
}

void mesh_render(mesh_t* mesh) {
    glDrawArrays(GL_TRIANGLES, 0, mesh->tri_cnt * 3);
}

void mesh_unbind() {
    glBindVertexArray(0);
}

void mesh_unload(mesh_t* mesh) {
    glDeleteVertexArrays(1, &mesh->vao);
    glDeleteBuffers(3, mesh->buffers);
    free(mesh);
}













