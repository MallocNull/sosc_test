#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define MAX_LINE_LEN 2048

int end_chk;

typedef struct list_t list_t;
typedef struct list_t {
    list_t* next;

    float data[3];
    uint32_t faces[3][3];
};

list_t* list_create() {
    list_t* list = malloc(sizeof(list_t));
    list->next = NULL;

    return list;
}

void list_destroy(list_t* list) {
    list_t* next = list->next;
    free(list);

    if(next != NULL)
        list_destroy(next);
}

#define ISWP(X) swap(X, 4)
void swap(uint8_t* in, int length) {
    if(end_chk)
        return;

    uint8_t tmp[8] = { 0 };
    for(int i = 0; i < length; ++i)
        tmp[i] = in[length - i - 1];
    memcpy(in, tmp, length);
}

void write(FILE* fp, const uint8_t* data, int length) {
    uint8_t tmp[8] = { 0 };
    memcpy(tmp, data, length);

    swap(tmp, length);
    fwrite(tmp, 1, length, fp);
}

int main(int argc, char* argv[]) {
    if(sizeof(float) != 4) {
        printf("Operating system must define floats as four bytes.");
        return -1;
    }

    if(argc < 2) {
        printf("You must pass the OBJ file to be converted.");
        return -1;
    }

    FILE* fpi = fopen(argv[1], "rb");
    if(fpi == NULL) {
        printf("File '%s' does not exist.", argv[1]);
        return -1;
    }

    char* out;
    if(argc >= 3)
        out = argv[2];
    else {
        out = malloc(sizeof(char) * strlen(argv[1]) + 5);
        strcpy(out, argv[1]);

        char* ext = NULL;
        if((ext = strrchr(out, '.')) == NULL)
            ext = out + strlen(out);

        strcpy(ext, ".rbm");
    }

    FILE* fpo = fopen(out, "wb");
    if(fpo == NULL) {
        printf("Could not write to file '%s'.", out);

        fclose(fpi);
        free(out);
        return -1;
    }

    // 0 for big endian, 1 for little endian
    uint16_t test = 0xB00B;
    end_chk = ((uint8_t*)&test)[0] == 0x0B;

    uint32_t vertex_cnt = 0,
             normal_cnt = 0,
             tex_cnt    = 0,
             face_cnt   = 0;

    // NOTE: to prevent writing special cases keep in mind
    // the first node in these lists is unused and is just
    // a blank root. do NOT include this node
    list_t  *verts = list_create(),
            *normals = list_create(),
            *texs = list_create(),
            *faces = list_create();

    list_t  *verts_at = verts,
            *normals_at = normals,
            *texs_at = texs,
            *faces_at = faces;

    char* chk;
    char line[MAX_LINE_LEN + 2];
    for(;;) {
        line[MAX_LINE_LEN] = '\n';

        chk = fgets(line, MAX_LINE_LEN + 2, fpi);
        if(feof(fpi) && chk == NULL)
            break;

        if(line[MAX_LINE_LEN] != '\n') {
            printf("Lines cannot exceed %i characters.", MAX_LINE_LEN);
            return -1;
        }

        list_t* next = list_create();
        int used = 0;

        if(strncmp("v ", line, 2) == 0) {
            used = 1;
            ++vertex_cnt;

            sscanf(line, "%*s %f %f %f",
                &next->data[0],
                &next->data[1],
                &next->data[2]
            );

            verts_at->next = next;
            verts_at = next;
        } else if(strncmp("vn ", line, 3) == 0) {
            used = 1;
            ++normal_cnt;

            sscanf(line, "%*s %f %f %f",
                &next->data[0],
                &next->data[1],
                &next->data[2]
            );

            normals_at->next = next;
            normals_at = next;
        } else if(strncmp("vt ", line, 3) == 0) {
            used = 1;
            ++tex_cnt;

            sscanf(line, "%*s %f %f",
                &next->data[0],
                &next->data[1]
            );

            texs_at->next = next;
            texs_at = next;
        } else if(strncmp("f ", line, 2) == 0) {
            used = 1;
            ++face_cnt;

            if(vertex_cnt > 0 && tex_cnt > 0 && normal_cnt > 0) {
                sscanf(line, "%*s %u/%u/%u %u/%u/%u %u/%u/%u",
                    &next->faces[0][0], &next->faces[0][1], &next->faces[0][2],
                    &next->faces[1][0], &next->faces[1][1], &next->faces[1][2],
                    &next->faces[2][0], &next->faces[2][1], &next->faces[2][2]
                );
            } else if(vertex_cnt > 0 && tex_cnt > 0 && normal_cnt == 0) {
                sscanf(line, "%*s %u/%u %u/%u %u/%u",
                    &next->faces[0][0], &next->faces[0][1],
                    &next->faces[1][0], &next->faces[1][1],
                    &next->faces[2][0], &next->faces[2][1]
                );
            } else if(vertex_cnt > 0 && tex_cnt == 0 && normal_cnt > 0) {
                sscanf(line, "%*s %u//%u %u//%u %u//%u",
                    &next->faces[0][0], &next->faces[0][2],
                    &next->faces[1][0], &next->faces[1][2],
                    &next->faces[2][0], &next->faces[2][2]
                );
            } else if(vertex_cnt > 0 && tex_cnt == 0 && normal_cnt == 0) {
                sscanf(line, "%*s %u %u %u",
                    &next->faces[0][0],
                    &next->faces[1][0],
                    &next->faces[2][0]
                );
            } else {
                printf("OBJ file must at least include vertices!");
                return -1;
            }

            faces_at->next = next;
            faces_at = next;
        }

        if(!used)
            free(next);
    }

    fclose(fpi);

    if(face_cnt == 0) {
        printf("OBJ file must have faces!");
        return -1;
    }

    uint8_t magic[] = {0xDE, 0xAF, 0xB0, 0x0B};
    fwrite(magic, 1, 4, fpo);

    write(fpo, (uint8_t*)&vertex_cnt, 4);
    write(fpo, (uint8_t*)&tex_cnt, 4);
    write(fpo, (uint8_t*)&normal_cnt, 4);
    write(fpo, (uint8_t*)&face_cnt, 4);

    list_t* at = verts->next;
    while(at != NULL) {
        write(fpo, (uint8_t*)&(at->data[0]), 4);
        write(fpo, (uint8_t*)&(at->data[1]), 4);
        write(fpo, (uint8_t*)&(at->data[2]), 4);

        at = at->next;
    }

    at = texs->next;
    while(at != NULL) {
        write(fpo, (uint8_t*)&(at->data[0]), 4);
        write(fpo, (uint8_t*)&(at->data[1]), 4);

        at = at->next;
    }

    at = normals->next;
    while(at != NULL) {
        write(fpo, (uint8_t*)&(at->data[0]), 4);
        write(fpo, (uint8_t*)&(at->data[1]), 4);
        write(fpo, (uint8_t*)&(at->data[2]), 4);

        at = at->next;
    }

    int face_length = 4;
    if(face_cnt <= 0xFFFF)
        face_length = 2;

    at = faces->next;
    while(at != NULL) {
        if(tex_cnt > 0 && normal_cnt > 0) {
            write(fpo, (uint8_t*)&(at->faces[0][0]), face_length);
            write(fpo, (uint8_t*)&(at->faces[0][1]), face_length);
            write(fpo, (uint8_t*)&(at->faces[0][2]), face_length);

            write(fpo, (uint8_t*)&(at->faces[1][0]), face_length);
            write(fpo, (uint8_t*)&(at->faces[1][1]), face_length);
            write(fpo, (uint8_t*)&(at->faces[1][2]), face_length);

            write(fpo, (uint8_t*)&(at->faces[2][0]), face_length);
            write(fpo, (uint8_t*)&(at->faces[2][1]), face_length);
            write(fpo, (uint8_t*)&(at->faces[2][2]), face_length);
        } else if(tex_cnt > 0 && normal_cnt == 0) {
            write(fpo, (uint8_t*)&(at->faces[0][0]), face_length);
            write(fpo, (uint8_t*)&(at->faces[0][1]), face_length);

            write(fpo, (uint8_t*)&(at->faces[1][0]), face_length);
            write(fpo, (uint8_t*)&(at->faces[1][1]), face_length);

            write(fpo, (uint8_t*)&(at->faces[2][0]), face_length);
            write(fpo, (uint8_t*)&(at->faces[2][1]), face_length);
        } else if(tex_cnt == 0 && normal_cnt > 0) {
            write(fpo, (uint8_t*)&(at->faces[0][0]), face_length);
            write(fpo, (uint8_t*)&(at->faces[0][2]), face_length);

            write(fpo, (uint8_t*)&(at->faces[1][0]), face_length);
            write(fpo, (uint8_t*)&(at->faces[1][2]), face_length);

            write(fpo, (uint8_t*)&(at->faces[2][0]), face_length);
            write(fpo, (uint8_t*)&(at->faces[2][2]), face_length);
        } else {
            write(fpo, (uint8_t*)&(at->faces[0][0]), face_length);

            write(fpo, (uint8_t*)&(at->faces[1][0]), face_length);

            write(fpo, (uint8_t*)&(at->faces[2][0]), face_length);
        }

        at = at->next;
    }

    list_destroy(verts);
    list_destroy(normals);
    list_destroy(texs);
    list_destroy(faces);

    fclose(fpo);
    free(out);

    return 0;
}