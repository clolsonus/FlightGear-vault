#define INCL_GPI
#define INCL_WIN
#include <stdlib.h>
#include <os2.h>
#include <mmioos2.h>
#include <dive.h>
#include <fourcc.h>
#include <string.h>
#include <stdio.h>
#include "dgraph.h"


#ifdef DEBUG
FILE *errfile;
#endif

static HPS dgraph_hps;
static HDC dgraph_hdc;

/*BYTE               abyVGADefault16Palette[] = {             */
/* B    G    R       O           B    G    R       O     */
/* 0,   0,   0, PC_RESERVED,   171,   0,   0, PC_RESERVED,
   0, 171,   0, PC_RESERVED,   171, 171,   0, PC_RESERVED,
   0,   0, 171, PC_RESERVED,   171,   0, 171, PC_RESERVED,
   0,  85, 171, PC_RESERVED,   171, 171, 171, PC_RESERVED,
  85,  85,  85, PC_RESERVED,   255,  85,  85, PC_RESERVED,
  85, 255,  85, PC_RESERVED,   255, 255,  85, PC_RESERVED,
  85,  85, 255, PC_RESERVED,   255,  85, 255, PC_RESERVED,
  85, 255, 255, PC_RESERVED,   255, 255, 255, PC_RESERVED
};
*/

/*HDIVE StartDive(DIVE_CAPS *pDiveCaps)
{
  HDIVE DiveHandle=1;
#ifdef DEBUG
  char text[100];
#endif

  pDiveCaps->ulStructLen=sizeof(DIVE_CAPS);

  if (!(pDiveCaps->pFormatData=malloc(100*sizeof(FOURCC)))) {
    WinMessageBox(HWND_DESKTOP,HWND_DESKTOP,"Unable to allocate memory!",
      "Fatal DIVE Error",0,MB_OK | MB_INFORMATION);
    DiveHandle=0;
  }

  pDiveCaps->ulFormatLength=100;

  if ( DiveQueryCaps(pDiveCaps,DIVE_BUFFER_SCREEN) ) {
    WinMessageBox(HWND_DESKTOP,HWND_DESKTOP,
      "Unable to query DIVE capabilities","Fatal DIVE Error",0,
      MB_OK | MB_INFORMATION);
    DiveHandle=0;
  }

  if ( pDiveCaps->ulDepth < 8 ) {
    WinMessageBox(HWND_DESKTOP,HWND_DESKTOP,
      "This program requires at least 256 colors to run!","Fatal DIVE Error",
      0,MB_OK | MB_INFORMATION);
    DiveHandle=0;
  }

  if (DiveHandle) {
    DiveHandle=0;
#ifdef DEBUG
    sprintf(text,"Here1! Dive Handle=%x",DiveHandle);
    DebugBox("Info",text);
#endif
    if (DiveOpen(&DiveHandle,FALSE,NULL)) {
      WinMessageBox(HWND_DESKTOP,HWND_DESKTOP,
        "Unable to open a DIVE Instance","Fatal DIVE Error",0,
        MB_OK | MB_INFORMATION);
        DiveHandle=0;
    }
#ifdef DEBUG
   sprintf(text,"Here2! Dive Handle=%x",DiveHandle);
   DebugBox("Info",text);
#endif
  }

  return (DiveHandle);
}
*/

/* ULONG EndDive(HDIVE DiveHandle, DIVE_CAPS *pDiveCaps)
{
  if (DiveHandle) {
    if (pDiveCaps->pFormatData) {
      free(pDiveCaps->pFormatData);
      pDiveCaps->pFormatData=NULL;
    }
    DiveClose(DiveHandle);
  }
  return (0);
}
*/

/*ULONG CreateDiveBuffer256(HDIVE DiveHandle, ULONG Width, ULONG Height, PBYTE **Lines)
{
  ULONG BufferNumber=0;
  char text[500];
  FOURCC ColorSpace=FOURCC_LUT8;
  PBYTE p,buffer=(PBYTE)calloc(Width*Height,sizeof(BYTE));


  register int numline=0;
  


  if (buffer && (*Lines=(PBYTE *)malloc(Height*sizeof(PBYTE)))
          && !(numline=DiveAllocImageBuffer(DiveHandle,&BufferNumber,ColorSpace,
                                   Width,Height,Width,buffer))) {
    for (p=(buffer+Width*(Height-1));p>=buffer;p-=Width) 
      (*Lines)[numline++]=p;
  } else {
    sprintf(text,"Unable to allocate image buffer\nDiveHandle=%x\n*Lines=%x\nnumline=%x",DiveHandle,*Lines,numline);
    DebugBox("Fatal DIVE Error",text);
    BufferNumber=0;
  }
#ifdef DEBUG
  sprintf(text,"Created Dive Buffer Number %d",BufferNumber);
  DebugBox("dgraph",text);
#endif
  return (BufferNumber);
}
*/

ULONG DestroyDiveBuffer(HDIVE DiveHandle, ULONG BufferNumber, PBYTE *Lines)
{
  DiveFreeImageBuffer(DiveHandle,BufferNumber);
#ifdef DEBUG
  DebugBox("dgraph","Destroying Dive Buffer!");
#endif
  free(*Lines);
  free(Lines);
  return (0);
}

/* VOID SetWindowSize(HWND hwndFrame,ULONG Width,ULONG Height)
{
  LONG BorderWidth    = (LONG)WinQuerySysValue(HWND_DESKTOP, SV_CXSIZEBORDER);
  LONG BorderHeight   = (LONG)WinQuerySysValue(HWND_DESKTOP, SV_CYSIZEBORDER);
  LONG TitleBarHeight = (LONG)WinQuerySysValue(HWND_DESKTOP, SV_CYTITLEBAR);
  LONG MenuHeight     = (LONG)WinQuerySysValue(HWND_DESKTOP, SV_CYMENU);
  LONG TotalWidth = Width+2*BorderWidth;
  LONG TotalHeight = Height+MenuHeight+TitleBarHeight+2*BorderHeight;
  WinSetWindowPos(hwndFrame,HWND_TOP, 
     (WinQuerySysValue(HWND_DESKTOP, SV_CXSCREEN) - TotalWidth ) / 2,
     (WinQuerySysValue(HWND_DESKTOP, SV_CYSCREEN) - TotalHeight ) / 2,
     TotalWidth,TotalHeight, SWP_MOVE|SWP_SIZE|SWP_SHOW|SWP_ACTIVATE);
}
*/

/* HPAL CreatePalette(HDIVE hDive, HWND hwndClient, 
                   ULONG **ppPaletteArray)
{
  ULONG ncolors=256;
  HPAL hpal;

  if (!(*ppPaletteArray)) {
    if (!(*ppPaletteArray=(ULONG *)malloc(ncolors*sizeof(RGB2)))) {
      DebugBox("Error!","Unable to allocate memory for palette!");
      return (0);
    } else {
      memcpy(*ppPaletteArray,abyVGADefault16Palette,64);
    }
  }

  hpal=SetPalette(hDive, hwndClient,*ppPaletteArray);
  return (hpal);
}
*/

/* HPAL SetPalette(HDIVE hDive, HWND hwndClient, ULONG *pPaletteArray)
{
  ULONG ncolors=256;
  SIZEL sizel={0,0};
  HPAL hpal=0;
  HAB hab;

  if (!dgraph_hdc) dgraph_hdc=WinOpenWindowDC(hwndClient);

#ifdef DEBUG
  DebugBox("dgraph","Setting Palette!");
#endif
  hab=WinQueryAnchorBlock(hwndClient);
  if (!dgraph_hps) {
    dgraph_hps=GpiCreatePS(hab,dgraph_hdc,&sizel,
       PU_PELS | GPIF_DEFAULT | GPIT_MICRO | GPIA_ASSOC );
  }
  hpal=GpiCreatePalette(hab,LCOL_PURECOLOR,LCOLF_CONSECRGB
                        ,ncolors,pPaletteArray);
  GpiSelectPalette(dgraph_hps,hpal);

  WinRealizePalette(hwndClient, dgraph_hps, &ncolors);
  DiveSetSourcePalette(hDive,0,256,(BYTE *)pPaletteArray);
  DiveSetDestinationPalette(hDive,0,256,0);
  return (hpal);
}
*/

/*VOID handle_WM_REALIZEPALETTE(HDIVE hDive) {
#ifdef DEBUG
  DebugBox("dgraph","Realizing Palette!");
#endif
  DiveSetDestinationPalette(hDive,0,256,0);
}
*/

/*VOID handle_WM_ACTIVATE(HWND hwndClient,SHORT WindowActive,HPAL hpal) 
{
  ULONG ncolors=256;
#ifdef DEBUG
  char text[500];
  sprintf(text,"Activating window\n hwnd=%d, Active=%d\n hpal=%d\n hps=%d",
  hwndClient,WindowActive,hpal,dgraph_hps);
  DebugBox("dgraph",text);
#endif
  if (dgraph_hps) {
    if (WindowActive && hpal)  {
      GpiSelectPalette(dgraph_hps,hpal);
      WinRealizePalette(hwndClient, dgraph_hps, &ncolors);
      WinInvalidateRect(hwndClient, NULL, FALSE);
    } else {
      GpiSelectPalette(dgraph_hps,NULLHANDLE);
      WinRealizePalette(hwndClient, dgraph_hps, &ncolors);
    }
  }
}
*/

/*VOID handle_WM_CLOSE(HWND hwndClient, HPAL hpal)
{
#ifdef DEBUG
    DebugBox("dgraph","Closing Window");
#endif
  handle_WM_ACTIVATE(hwndClient, 0, NULLHANDLE);
  if (hpal) GpiDeletePalette(hpal);
  if (dgraph_hps) GpiDestroyPS(dgraph_hps);
}
*/

/*VOID handle_WM_VRNENABLED(HWND hwnd, HDIVE hDive, UINT xsize, UINT ysize)
{
  static SETUP_BLITTER SetupBlitter;
  RGNRECT            rgnCtl;
  SWP swp;
  POINTL pointl;
  HRGN hrgn;
  RECTL rcls[50];
  HWND hwndFrame;
  
#ifdef DEBUG
  char text[500];
  sprintf(text,"Handling WM_VRNENABLED\n hwnd=%d\n hdive=%d\n buffer=(%d,%d)",hwnd,hDive,xsize,ysize);
  DebugBox("dgraph",text);
#endif
  hwndFrame=WinQueryWindow(hwnd,QW_PARENT);
  if ( dgraph_hps && (hrgn=GpiCreateRegion(dgraph_hps,0L,NULL)) ) {
    WinQueryVisibleRegion(hwnd,hrgn);
    rgnCtl.ircStart=0;
    rgnCtl.crc=50;
    rgnCtl.ulDirection  = 1;
    if ( GpiQueryRegionRects ( dgraph_hps, hrgn, NULL, &rgnCtl, rcls) ) {
      WinQueryWindowPos (hwnd, &swp );
      pointl.x=swp.x;
      pointl.y=swp.y;
      WinMapWindowPoints ( hwndFrame, HWND_DESKTOP, &pointl, 1 );
      
      SetupBlitter.ulStructLen=sizeof(SetupBlitter);
      SetupBlitter.fInvert=FALSE;
      SetupBlitter.fccSrcColorFormat=FOURCC_LUT8;
      SetupBlitter.ulSrcWidth=xsize;
      SetupBlitter.ulSrcHeight=ysize;
      SetupBlitter.ulSrcPosX=0;
      SetupBlitter.ulSrcPosY=0;
      SetupBlitter.ulDitherType=1;

      SetupBlitter.fccDstColorFormat = FOURCC_SCRN;
      SetupBlitter.ulDstWidth        = swp.cx;
      SetupBlitter.ulDstHeight       = swp.cy;
      SetupBlitter.lDstPosX          = 0;
      SetupBlitter.lDstPosY          = 0;
      SetupBlitter.lScreenPosX       = pointl.x;
      SetupBlitter.lScreenPosY       = pointl.y;
      SetupBlitter.ulNumDstRects     = rgnCtl.crcReturned;
      SetupBlitter.pVisDstRects      = rcls;
      DiveSetupBlitter (hDive, &SetupBlitter);
    } else {
      DiveSetupBlitter (hDive, NULL);
    } 
    GpiDestroyRegion(dgraph_hps,hrgn);
  }
}
*/
/*
VOID handle_WM_PAINT(HWND hwnd, HDIVE hDive, ULONG DiveBufferNumber)
{
  HPS hps;
#ifdef DEBUG
  char text[500];
  sprintf(text,"Handling WM_PAINT\n hwnd=%d\n hDive=%d\n BufferNumber=%d",
    hwnd,hDive,DiveBufferNumber);
  DebugBox("dgraph",text);
#endif
  hps=WinBeginPaint(hwnd, NULLHANDLE, NULL);
  DiveBlitImage(hDive, DiveBufferNumber, DIVE_BUFFER_SCREEN);
  WinEndPaint(hps);
}
*/

/*VOID handle_WM_VRNDISABLED(HDIVE hDive)
{
#ifdef DEBUG
  DebugBox("dgraph","Handling WM_VRNDISABLED");
#endif
  DiveSetupBlitter (hDive, 0);
}
*/


VOID CyclePalette(HDIVE hDive, HWND hwndClient, ULONG *pPaletteArray)
{
  register ULONG temp=pPaletteArray[0];
  memmove(&pPaletteArray[0],&pPaletteArray[1],256*sizeof(ULONG));
  pPaletteArray[255]=temp;
  SetPalette(hDive,hwndClient,pPaletteArray);
}



