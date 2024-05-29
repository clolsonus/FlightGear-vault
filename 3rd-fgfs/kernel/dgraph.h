#ifndef _DGRAPH_H_
#define _DGRAPH_H_

#ifndef DEBUG

#define DebugBox(title, text) WinMessageBox( HWND_DESKTOP, HWND_DESKTOP, \
                               (PSZ)text, (PSZ)title, 0, \
                               MB_OK | MB_INFORMATION | MB_MOVEABLE)
#else

#define DebugBox(title,text) if (!errfile) errfile=fopen("debug.out","w");  \
  fprintf(errfile,"%s-- %s\n",title,text); \
  fflush(errfile)

#endif

typedef struct _UINT_2D {
  UINT x;
  UINT y;
} UINT_2D;

typedef struct _FLOAT_2D {
  float x;
  float y;
} FLOAT_2D;

extern BYTE abyVGADefault16Palette[];
extern HPS dgraph_hps;
extern HDC dgraph_hdc;
extern HDIVE hDive;
typedef UINT HDGRAPH;

#define GRGB_GRAY                    (0)
#define GRGB_RED                     (64)
#define GRGB_GREEN                   (128)
#define GRGB_BLUE                    (192)
#define GRGB_MAX                     (255)

#define BGCRMYG_BLUE                 (0)
#define BGCRMYG_GREEN                (36)
#define BGCRMYG_CYAN                 (72)
#define BGCRMYG_RED                  (108)
#define BGCRMYG_MAGENTA              (144)
#define BGCRMYG_YELLOW               (180)
#define BGCRMYG_GRAY                 (216)

#define RGBP_BLUE                    (0)
#define RGBP_GREEN                   (85)
#define RGBP_RED                     (170)

#define ROUND(x) ((x) > 0 ? (int)((x)+0.5) : (int)((x)-0.5))
#define MAX(x,y) ((x) > (y) ? (x) : (y))
#define MIN(x,y) ((x) < (y) ? (x) : (y))
#define ABS(x) ((x) < (0) ? (-1*(x)) : (x))

HDIVE StartDive(DIVE_CAPS *pDiveCaps);
ULONG EndDive(HDIVE DiveHandle, DIVE_CAPS *pDiveCaps);
ULONG CreateDiveBuffer256(HDIVE DiveHandle, ULONG Width, ULONG Height, PBYTE **Lines);
ULONG DestroyDiveBuffer(HDIVE DiveHandle, ULONG BufferNumber, PBYTE *Lines);
VOID SetWindowSize(HWND hwndFrame,ULONG Width,ULONG Height);
HPAL CreatePalette(HDIVE hDive, HWND hwndClient, ULONG **ppPaletteArray);
HPAL SetPalette(HDIVE hDive, HWND hwndClient, ULONG *pPaletteArray);
VOID CyclePalette(HDIVE hDive, HWND hwndClient, ULONG *pPaletteArray);
ULONG *GRGB_Palette(void);
ULONG *BGCRMYG_Palette(void) ;
ULONG *RGBPalette( void );
ULONG *World1Palette(void);
ULONG *World2Palette(void);
inline VOID hline(UINT x1, UINT x2, UINT y, UINT color, BYTE *Lines[]);
inline VOID hlineb(UINT x1, UINT x2, UINT y, UINT color, BYTE *Lines[], UINT xres, UINT yres);
inline VOID vline(UINT x, UINT y1, UINT y2, UINT color, BYTE *Lines[]);
inline VOID vlineb(UINT x, UINT y1, UINT y2, UINT color, BYTE *Lines[], UINT xres, UINT yres);
VOID handle_WM_REALIZEPALETTE(HDIVE hDive) ;
VOID handle_WM_ACTIVATE(HWND hwndClient,SHORT WindowActive,HPAL hpal) ;
VOID handle_WM_CLOSE(HWND hwndClient, HPAL hpal);
VOID handle_WM_VRNENABLED(HWND hwndFrame, HDIVE hDive, UINT xsize, UINT ysize);
VOID handle_WM_PAINT(HWND hwndFrame, HDIVE hDive, ULONG DiveBufferNumber);
VOID handle_WM_VRNDISABLED(HDIVE hDive);
inline VOID line(UINT x1, UINT y1, UINT x2, UINT y2, UINT color, BYTE *Lines[]);
inline VOID xlinex(UINT x1,UINT y1,INT deltax,INT deltay,UINT color,BYTE *Lines[]);
inline VOID yliney(UINT x1,UINT y1,INT deltax,INT deltay,UINT color,BYTE *Lines[]);
#endif
