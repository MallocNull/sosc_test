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
    /*if(end_chk)
        return;*/

    uint8_t tmp[8] = { 0 };
    for(int i = 0; i < length; ++i)
        tmp[i] = in[length - i - 1];
    memcpy(in, tmp, length);
}

int main(int argc, char* argv[]) {
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


    }

    fclose(fpi);
    fclose(fpo);
    free(out);

    return 0;
}