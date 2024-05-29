/*         ______   ___    ___ 
 *        /\  _  \ /\_ \  /\_ \ 
 *        \ \ \L\ \\//\ \ \//\ \      __     __   _ __   ___ 
 *         \ \  __ \ \ \ \  \ \ \   /'__`\ /'_ `\/\`'__\/ __`\
 *          \ \ \/\ \ \_\ \_ \_\ \_/\  __//\ \L\ \ \ \//\ \L\ \
 *           \ \_\ \_\/\____\/\____\ \____\ \____ \ \_\\ \____/
 *            \/_/\/_/\/____/\/____/\/____/\/___L\ \/_/ \/___/
 *                                           /\____/
 *                                           \_/__/
 *      By Shawn Hargreaves,
 *      1 Salisbury Road,
 *      Market Drayton,
 *      Shropshire,
 *      England, TF9 1AJ.
 *
 *      Main header file for the Allegro library.
 *      This should be included by everyone and everything.
 *
 *      See readme.txt for copyright information.
 */



#ifndef ALLEGRO_H
#define ALLEGRO_H

#ifdef __cplusplus
extern "C" {
#endif


#define VERSION_STR     "2.0 beta"
#define DATE_STR        "1996"



/*******************************************/
/************ Some global stuff ************/
/*******************************************/

#ifndef TRUE 
#define TRUE         -1
#define FALSE        0
#endif

#ifndef NULL 
#define NULL         ((void *)0L)
#endif

#ifndef MIN
#define MIN(x,y)     (((x) < (y)) ? (x) : (y))
#define MAX(x,y)     (((x) > (y)) ? (x) : (y))
#define MID(x,y,z)   MAX(x, MIN(y, z))
#endif

#ifndef ABS
#define ABS(x)       (((x) >= 0) ? (x) : (-x))
#endif

#ifndef SGN
#define SGN(x)       (((x) >= 0) ? 1 : -1)
#endif

extern char allegro_error[];

int allegro_init();
void allegro_exit();

int lock_memory(void *ptr, int size, int code);

struct BITMAP;
int lock_bitmap(struct BITMAP *bmp);

struct SAMPLE;
void lock_sample(struct SAMPLE *spl);

struct MIDI;
void lock_midi(struct MIDI *midi);

#define END_OF_FUNCTION(x) void x##_end() { }

#define LOCK_VARIABLE(x)   lock_memory((void *)&x, sizeof(x), FALSE)
#define LOCK_FUNCTION(x)   lock_memory(x, (int)x##_end - (int)x, TRUE)



/****************************************/
/************ Mouse routines ************/
/****************************************/

int install_mouse();
void remove_mouse();

extern volatile int mouse_x;
extern volatile int mouse_y;
extern volatile int mouse_b;

void set_mouse_sprite(struct BITMAP *sprite);
void show_mouse(struct BITMAP *bmp);
void position_mouse(int x, int y);



/****************************************/
/************ Timer routines ************/
/****************************************/

#define TIMERS_PER_SECOND     1193181L
#define SECS_TO_TIMER(x)      ((long)x * TIMERS_PER_SECOND)
#define MSEC_TO_TIMER(x)      ((long)x * (TIMERS_PER_SECOND / 1000))
#define BPS_TO_TIMER(x)       (TIMERS_PER_SECOND / (long)x)
#define BPM_TO_TIMER(x)       ((60 * TIMERS_PER_SECOND) / (long)x)

int install_timer();
void remove_timer();

int install_int_ex(void (*proc)(), long speed);
int install_int(void (*proc)(), long speed);
void remove_int(void (*proc)());
void rest(long time);




/*******************************************/
/************ Keyboard routines ************/
/*******************************************/

int install_keyboard();
void remove_keyboard();

extern volatile char key[128];

int readkey();
int keypressed();
void clear_keybuf();

#define KEY_ESC         1        /* keyboard scan codes */
#define KEY_1           2 
#define KEY_2           3 
#define KEY_3           4
#define KEY_4           5
#define KEY_5           6
#define KEY_6           7
#define KEY_7           8
#define KEY_8           9
#define KEY_9           10
#define KEY_0           11
#define KEY_MINUS       12
#define KEY_EQUALS      13
#define KEY_BACKSPACE   14
#define KEY_TAB         15 
#define KEY_Q           16
#define KEY_W           17
#define KEY_E           18
#define KEY_R           19
#define KEY_T           20
#define KEY_Y           21
#define KEY_U           22
#define KEY_I           23
#define KEY_O           24
#define KEY_P           25
#define KEY_OPENBRACE   26
#define KEY_CLOSEBRACE  27
#define KEY_ENTER       28
#define KEY_CONTROL     29
#define KEY_A           30
#define KEY_S           31
#define KEY_D           32
#define KEY_F           33
#define KEY_G           34
#define KEY_H           35
#define KEY_J           36
#define KEY_K           37
#define KEY_L           38
#define KEY_COLON       39
#define KEY_QUOTE       40
#define KEY_TILDE       41
#define KEY_LSHIFT      42
#define KEY_Z           44
#define KEY_X           45
#define KEY_C           46
#define KEY_V           47
#define KEY_B           48
#define KEY_N           49
#define KEY_M           50
#define KEY_COMMA       51
#define KEY_STOP        52
#define KEY_SLASH       53
#define KEY_RSHIFT      54
#define KEY_ASTERISK    55
#define KEY_ALT         56
#define KEY_SPACE       57
#define KEY_CAPSLOCK    58
#define KEY_F1          59
#define KEY_F2          60
#define KEY_F3          61
#define KEY_F4          62
#define KEY_F5          63
#define KEY_F6          64
#define KEY_F7          65
#define KEY_F8          66
#define KEY_F9          67
#define KEY_F10         68
#define KEY_NUMLOCK     69
#define KEY_SCRLOCK     70
#define KEY_HOME        71
#define KEY_UP          72
#define KEY_PGUP        73
#define KEY_MINUS_PAD   74
#define KEY_LEFT        75
#define KEY_5_PAD       76
#define KEY_RIGHT       77
#define KEY_PLUS_PAD    78
#define KEY_END         79
#define KEY_DOWN        80
#define KEY_PGDN        81
#define KEY_INSERT      82
#define KEY_DEL         83
#define KEY_F11         87
#define KEY_F12         88



/******************************************/
/************ Pallete routines ************/
/******************************************/

typedef struct RGB
{
   unsigned char r, g, b;
   unsigned char filler;
} RGB;

#define PAL_SIZE     256

typedef RGB PALLETE[PAL_SIZE];

extern RGB black_rgb;
extern PALLETE black_pallete, desktop_pallete;

void vsync();
void set_color(int index, RGB *p);
void set_pallete(RGB *p);
void get_color(int index, RGB *p);
void get_pallete(RGB *p);
void fade_in(RGB *p, int speed);
void fade_out(int speed);



/************************************************/
/************ Screen/bitmap routines ************/
/************************************************/

#define GFX_TEXT              -1
#define GFX_AUTODETECT        0
#define GFX_VGA               1
#define GFX_VESA1             2
#define GFX_VESA2B            3
#define GFX_VESA2L            4
#define GFX_CIRRUS64          5
#define GFX_CIRRUS54          6
#define GFX_S3                7
#define GFX_ET3000            8
#define GFX_ET4000            9


typedef struct GFX_DRIVER     /* creates and manages the screen bitmap */
{
   char *name;                /* driver name */
   char *desc;                /* description (VESA version, etc) */
   struct BITMAP *(*init)(int w, int h, int v_w, int v_h);
   void (*exit)(struct BITMAP *b);
   void (*scroll)(int x, int y);
   int w, h;                  /* physical (not virtual!) screen size */
   int linear;                /* true if video memory is linear */
   long bank_size;            /* bank size, in bytes */
   long bank_gran;            /* bank granularity, in bytes */
   long vid_mem;              /* video memory size, in bytes */
} GFX_DRIVER;


extern GFX_DRIVER gfx_vga, gfx_vesa_1, gfx_vesa_2b, gfx_vesa_2l, gfx_cirrus64, 
		  gfx_cirrus54, gfx_s3, gfx_et3000, gfx_et4000;

extern GFX_DRIVER *gfx_driver;


typedef struct BITMAP         /* a bitmap structure */
{
   int w, h;                  /* width and height in pixels */
   int clip;                  /* flag if clipping is turned on */
   int cl, cr, ct, cb;        /* clip left, right, top and bottom values */
   void *dat;                 /* the memory we allocated for the bitmap */
   int seg;                   /* bitmap segment */
   void (*write_bank)();      /* write bank selector, see bank.s */
   void (*read_bank)();       /* read bank selector, see bank.s */
   unsigned char *line[0];    /* pointers to the start of each line */
} BITMAP;


extern BITMAP *screen;

#define SCREEN_W     (gfx_driver ? gfx_driver->w : 0)
#define SCREEN_H     (gfx_driver ? gfx_driver->h : 0)

#define VIRTUAL_W    (screen ? screen->w : 0)
#define VIRTUAL_H    (screen ? screen->h : 0)

int set_gfx_mode(int card, int w, int h, int v_w, int v_h);
int scroll_screen(int x, int y);

BITMAP *create_bitmap(int width, int height);
void destroy_bitmap(BITMAP *bitmap);

void set_clip(BITMAP *bitmap, int x1, int y1, int x2, int y2);



/******************************************************/
/************ Graphics and sprite routines ************/
/******************************************************/

void xor_mode(int xor);
int  getpixel(BITMAP *bmp, int x, int y);
void putpixel(BITMAP *bmp, int x, int y, int color);
void vline(BITMAP *bmp, int x, int y1, int y2, int color);
void hline(BITMAP *bmp, int x1, int y, int x2, int color);
void do_line(BITMAP *bmp, int x1, int y1, int x2, int y2, int d, void (*proc)(BITMAP *, int, int, int));
void line(BITMAP *bmp, int x1, int y1, int x2, int y2, int color);
void triangle(BITMAP *bmp, int x1, int y1, int x2, int y2, int x3, int y3, int color);
void polygon(BITMAP *bmp, int color, int vertices, ...);
void rect(BITMAP *bmp, int x1, int y1, int x2, int y2, int color);
void rectfill(BITMAP *bmp, int x1, int y1, int x2, int y2, int color);
void do_circle(BITMAP *bmp, int x, int y, int radius, int d, void (*proc)(BITMAP *, int, int, int));
void circle(BITMAP *bmp, int x, int y, int radius, int color);
void circlefill(BITMAP *bmp, int x, int y, int radius, int color);
void draw_sprite(BITMAP *bmp, BITMA