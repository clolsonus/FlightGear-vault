#include <stdio.h>

#include "Display.h"


// Constructor
Display::Display(void) {
    printf("Creating a new instance of class Display\n");
}


int Display::Init() {
    vga_init();

    VGAMODE = vga_getdefaultmode();
    if (VGAMODE == -1) {
        VGAMODE = G320x200x256; /* Default mode. */
    }

    if (!vga_hasmode(VGAMODE)) {
        printf("Mode not available.\n");
        return(0);
    }

    VIRTUAL = 0;                /* No virtual screen. */
    if (vga_getmodeinfo(VGAMODE)->colors == 16 ||
        (vga_getmodeinfo(VGAMODE)->flags & IS_MODEX))
        /* These modes are supported indirectly by vgagl. */
        VIRTUAL = 1;

    if (VIRTUAL) {
        /* Create virtual screen. */
        gl_setcontextvgavirtual(VGAMODE);
        backscreen = gl_allocatecontext();
        gl_getcontext(backscreen);
    }

    vga_setmode(VGAMODE);

    gl_setcontextvga(VGAMODE);  /* Physical screen context. */
    physicalscreen = gl_allocatecontext();
    gl_getcontext(physicalscreen);
    if (COLORS == 256) {
        // gl_setrgbpalette();
    }

    return(1);
}


int Display::Get_XSize(void) {
    return(WIDTH);
}


int Display::Get_YSize(void) {
    return(HEIGHT);
}


// Destructor
Display::~Display(void) {
    printf("Destroying an instance of class Display\n");
}

