// main.cc
//
// This is the main file.
//
// $Id: graph_main.c,v 1.19 1994/03/15 03:43:02 curt Exp $
// (Log is kept at end of this file)
//


#include "ArmScreen.h"
#include "Trans.h"


// process the commands from stdin
void process_cmds(ArmScreen S, Trans T) {
    char command[256];
    int at_end = 0;

    while ( !at_end ) {
	// Get and parse command
	gets( command );

	if ( strlen(command) ) {
	    // Basic commands
	    if ( strcmp(command, "exit") == 0 ) {
		at_end = 1;
	    } else {
		// try and parse as a set of angles
	    }
	}
    }
}


// Ok, let's get started ...
int main( int argc, char **argv ) {
    ArmScreen S(argc, argv, "xarmsim", "xarmsim");
    Trans T;
    double wp[3] = {0.0, 0.0, 0.0};
    double vp[2];
    int sp1[2], sp2[2];

    // Set up and initialize graphics system
    S.Clear();

    // Set up and initialize the transformation matrix
    T.Set_x_and_y(X_SIZE, Y_SIZE);
    T.CalcTransMatrix();
    T.CalcProjection(wp, vp);
    printf("(%.2f, %.2f)\n", vp[0], vp[1]);    
    T.MapToScreen(vp, sp1);
    printf("(%d, %d)\n", sp1[0], sp1[1]);    

    // Now do whatever we are told
    process_cmds(S, T);

    printf("End of simulation ... exiting.\n");
    
    return( 0 );
}


//
// $Log: graph_main.c,v $
//

