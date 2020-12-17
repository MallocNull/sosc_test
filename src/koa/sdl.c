#include "sdl.h"

void get_pixel(SDL_Color* out, SDL_Surface* img, int x, int y) {
    int bpp = img->format->BytesPerPixel;
    uint8_t* ptr = (uint8_t*)img->pixels + y * img->pitch + x * bpp;
    Uint32 pixel;

    switch(bpp) {
    case 1:
        pixel = *ptr;
        break;
    case 2:
        pixel = *(Uint16*)ptr;
        break;
    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
            pixel = ptr[0] << 16 | ptr[1] << 8 | ptr[2];
        else
            pixel = ptr[0] | ptr[1] << 8 | ptr[2] << 16;
        break;
    case 4:
        pixel = *(Uint32*)ptr;
        break;
    default:
        pixel = 0;
    }

    SDL_GetRGBA(pixel, img->format, &out->r, &out->g, &out->b, &out->a);
}
