#include "platform.h"

#ifdef TARGET_LINUX
typedef struct {
    u8 r;
    u8 g;
    u8 b;
} color;
#else
typedef u16 color;
#endif


void draw_line(int y, u8* pixels) {
    color linebuffer[256];
    for (int x = 0; x < 256; x++) {
        u8 clr = pixels[x];
        u8 r = clr & 0x3;
        u8 g = (clr & 0xC) >> 2;
        u8 b = (clr & 0x30) >> 4;

        #ifdef TARGET_LINUX
        linebuffer[x].r = (r * 255 / 3);
        linebuffer[x].g = (g * 255 / 3);
        linebuffer[x].b = (b * 255 / 3);
        #else
        u8 _r = (r * 31 / 3) & 0x1F;
        u8 _g = (g * 63 / 3) & 0x3F;
        u8 _b = (b * 31 / 3) & 0x1F;
        linebuffer[x] = (_r << 11) | (_g << 5) | _b; 
        #endif
    }
    
    #ifdef TARGET_LINUX
    for (int x = 0; x < 256; x++) {
        DrawPixel(x*2, y*2, (Color){ linebuffer[x].r, linebuffer[x].g, linebuffer[x].b, 255 });
        DrawPixel(x*2+1, y*2, (Color){ linebuffer[x].r, linebuffer[x].g, linebuffer[x].b, 255});
        DrawPixel(x*2, y*2+1, (Color){ linebuffer[x].r, linebuffer[x].g, linebuffer[x].b, 255});
        DrawPixel(x*2+1, y*2+1, (Color){ linebuffer[x].r, linebuffer[x].g, linebuffer[x].b, 255});
    }
    #else
    eadk_display_push_rect((eadk_rect_t){0, y, 256, 1}, linebuffer);
    #endif
}
