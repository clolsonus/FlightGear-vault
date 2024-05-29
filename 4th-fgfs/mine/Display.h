#include <vga.h>
#include <vgagl.h>


class Display {

    int VGAMODE;
    int VIRTUAL;

    GraphicsContext *backscreen;
    GraphicsContext *physicalscreen;

  public:

    Display(void);

    Init();

    int Get_XSize(void);
    int Get_YSize(void);

    ~Display(void);
};
