// ArmScreen.cc
//
// This file contains code to draw on the screen using the Graphics class
// routines.
//
// $Id: graph_main.c,v 1.19 1994/03/15 03:43:02 curt Exp $
// (Log is kept at end of this file) 
//


#include "ArmScreen.h"


// Constructor
ArmScreen::ArmScreen(int argc, char **argv, char win_name[], 
		     char icon_name[]) {
    printf("Creating a new instance of class ArmScreen\n");
    G.init_graphics(argc, argv, win_name, icon_name);
}


void ArmScreen::Clear() {
    // G.clear_window();
    printf("Clearing display\n");
    printf("(%d,%d) -> (%d,%d)\n",0, (int)(Y_SIZE/2), 
	   X_SIZE - 1, (int)(Y_SIZE/2));
    printf("(%d,%d) -> (%d,%d)\n",(int)(X_SIZE/2), 0, 
	   (int)(X_SIZE/2), Y_SIZE - 1, 1);
    G.draw_line(0, (int)(Y_SIZE/2), X_SIZE - 1, (int)(Y_SIZE/2), 1);
    G.draw_line((int)(X_SIZE/2), 0, (int)(X_SIZE/2), Y_SIZE - 1, 1);
    G.flush_graphics();
}


// Destructor
ArmScreen::~ArmScreen() {
    printf("Destroying an instance of class ArmScreen\n");
}


//
// $Log: graph_main.c,v $
//
