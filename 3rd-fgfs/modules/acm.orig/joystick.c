/*
 *  acm : an aerial combat simulator for X
 *  Copyright (C) 1991-1994  Riley Rainey
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 dated June, 1991.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program;  if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave., Cambridge, MA 02139, USA.
 */

/*
 *  Colorado Spectrum Workstation Gameport(TM)
 *
 *  Interface Notes by Riley Rainey (rainey@netcom.com)
 *
 *  The Colorado Spectrum Workstation Gameport allows you to connect
 *  a PC joystick to a serial port on your Unix workstation.  It presents
 *  a DB-9 plug that can be attached directly into any system that has a
 *  PC-style 9 pin serial port.  I created a simple adapter cable to convert a
 *  Sparc IPX DIN-8 serial connector to the 9-pin that this unit requires.
 *
 *  The Workstation Gameport costs about $100.
 *  Colorado Spectrum accepts credit card phone orders: +1 (970) 225-6929
 *
 *  The workstation gameport emits asynchronous six byte sequences
 *  up to 30 times per second.  If the state of switches and joystick pots
 *  does not change, no packet is emitted.  The data rate is 9600 bps;
 *  eight data bits and two stop bits.
 *
 *  Byte  Contents
 *  ----  ----------------------------------------------
 *   0    Sync Byte (always zero)
 *   1    Switches  (J2s2, J2s1, J1s2, J1s1, 0, 0, 0, 0)
 *   2    J1x
 *   3    J1y
 *   4    J2x
 *   5    J2y
 *
 *  The driver (this module) must ensure that both DTR and RTS
 *  are raised to supply power to the joystick adapter.
 */

#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

static int min[4] =
{0, 0, 0, 0};
static int max[4] =
{255, 255, 255, 255};
static int home[4] =
{128, 128, 128, 128};
static int value[5] =
{128, 128, 128, 128, 0};

static char *joystick_port = NULL;

void
SetJoystickPort (char * name)
{
    joystick_port = name;
}

void
CalibrateJoystick()
{
    int i;
    for (i = 0; i < 4; ++i) {
	home[i] = value[i];
    }
}

void
GetJoystickPosition(double *x1, double *y1,
		    double *x2, double *y2, int *switches)
{
    *x1 = (double) (value[0] - home[0]) / (max[0] - home[0]);
    if (*x1 < -1.0) {
	*x1 = -1.0;
    }
    *y1 = (double) (value[1] - home[1]) / (max[1] - home[1]);
    if (*y1 < -1.0) {
	*y1 = -1.0;
    }
    *x2 = (double) (value[2] - home[2]) / (max[2] - home[2]);
    if (*x2 < -1.0) {
	*x2 = -1.0;
    }
    *y2 = (double) (value[3] - home[3]) / (max[3] - home[3]);
    if (*y2 < -1.0) {
	*y2 = -1.0;
    }
    *switches = value[4];
}

/*
 *  ProcessJoystickInput()
 *
 *  This procedure should be called just before you call GetJoystickInput().
 *  In processes any pending input from the Workstation Gameport.
 */

static int init = 0, state;

int
ProcessJoystickInput()
{
    static int fd;
    static int x1, y1, x2, switches;
    int on = 1;
    int flags, n, i, updated = 0;
    unsigned char bytes[512], *p;

    if (init < 0) {
	return init;
    } else if (init == 0) {
	struct termios term;

	init = -1;

	if (!joystick_port) {
	    return init;
	}

	if ((fd = open(joystick_port, O_RDWR)) < 0) {
	    perror("joystick port open failed");
	    return -1;
	}

/*
 *  Set POSIX non-blocking I/O
 */

	if ((flags = fcntl(fd, F_GETFL, 0)) == -1) {
	    perror("F_GETFL failed");
	    return -1;
	}
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) != 0) {
	    perror("F_SETFL failed");
	    return -1;
	}

/*
 *  Assert both DTR and RTS (these pins supply power to
 *  the Workstation Gameport).
 */

#ifdef TIOCM_RTS
	flags = TIOCM_RTS | TIOCM_DTR | TIOCM_LE;
	if (ioctl(fd, TIOCMSET, &flags) == -1) {
	    perror("ioctl failed");
	    return -1;
	}
#endif

/*
 *  Condition the TTY line to talk to the converter
 */

	if (tcgetattr(fd, &term) != 0) {
	    perror("tcgetattr failed");
	    return -1;
	}
	term.c_iflag = IGNBRK | IGNPAR;
	term.c_oflag = 0;
	term.c_cflag = CLOCAL | CS8 | CSTOPB | CREAD;
	term.c_lflag &= ~(ECHO | ICANON);
	for (i = 0; i < NCCS; ++i) {
	    term.c_cc[i] = -1;
	}
	term.c_cc[VMIN] = 1;
	term.c_cc[VTIME] = 0;
	cfsetospeed(&term, B9600);
	cfsetispeed(&term, B9600);
	if (tcsetattr(fd, TCSAFLUSH, &term) != 0) {
	    perror("tcsetattr failed");
	    return -1;
	}
	state = 0;
	init = 1;
    }
/*
 *  Read a whole bunch of characters from the tty
 */

    n = read(fd, bytes, sizeof bytes);

/*
 *  And then process them ...
 */

    for (p = bytes; n > 0; --n) {

	switch (state) {

	case 0:
	    if (*p == 0) {
		state = 1;
	    }
	    break;

	case 1:
	    if (*p & 0x0f == 0 || 1) {
		switches = *p >> 4;
		state = 2;
	    } else {
		state = 0;
	    }
	    break;

	case 2:
	    x1 = *p;
	    state = 3;
	    break;

	case 3:
	    y1 = *p;
	    state = 4;
	    break;

	case 4:
	    x2 = *p;
	    state = 5;
	    break;

	case 5:
	    state = 0;
	    value[0] = x1;
	    value[1] = y1;
	    value[2] = x2;
	    value[3] = *p;
	    value[4] = switches;
	    updated = 1;
	    break;
	}
	++p;
    }
    return updated;
}
