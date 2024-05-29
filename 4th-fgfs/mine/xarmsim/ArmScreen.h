// ArmScreen.h
//
// This file contains code to draw on the screen using the Graphics class
// routines.
//
// $Id: graph_main.c,v 1.19 1994/03/15 03:43:02 curt Exp $
// (Log is kept at end of this file) 
//

#ifndef ARMSCREEN_H
#define ARMSCREEN_H


#include "Graphics.h"


class ArmScreen {
    // These routines paint the screen
      
    Graphics G;
    
  public:
    
    ArmScreen(int argc, char **argv, char win_name[], char icon_name[]);
    
    // Clear the screen
    void Clear();
    
    ~ArmScreen();
};


#endif ARMSCREEN_H

//
// $Log: graph_main.c,v $
//
