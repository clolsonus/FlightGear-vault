#include <stdio.h>

#include "Graphics.h"

Graphics G;

main(int argc, char **argv) {
    char s[256];

    G.init_graphics(argc, argv, "test", "test");
    G.draw_line(5, 5, 200, 200, 0);
    G.flush_graphics();

    for ( ; ; ) {
	printf("looping:\n");
	gets(s);
        G.flush_graphics();
    }
}
